#include "websocket_protocol.h"
#include "board.h"
#include "system_info.h"
#include "application.h"
#include "settings.h"

#include <cstring>
#include <cJSON.h>
#include <esp_log.h>
#include <arpa/inet.h>
#include "assets/lang_config.h"

#define TAG "WS"

WebsocketProtocol::WebsocketProtocol() {
    event_group_handle_ = xEventGroupCreate();
}

WebsocketProtocol::~WebsocketProtocol() {
    vEventGroupDelete(event_group_handle_);
}

bool WebsocketProtocol::Start() {
    // Only connect to server when audio channel is needed
    return true;
}

bool WebsocketProtocol::SendAudio(std::unique_ptr<AudioStreamPacket> packet) {
    if (websocket_ == nullptr || !websocket_->IsConnected()) {
        return false;
    }

    if (version_ == 2) {
        std::string serialized;
        serialized.resize(sizeof(BinaryProtocol2) + packet->payload.size());
        auto bp2 = (BinaryProtocol2*)serialized.data();
        bp2->version = htons(version_);
        bp2->type = 0;
        bp2->reserved = 0;
        bp2->timestamp = htonl(packet->timestamp);
        bp2->payload_size = htonl(packet->payload.size());
        memcpy(bp2->payload, packet->payload.data(), packet->payload.size());

        return websocket_->Send(serialized.data(), serialized.size(), true);
    } else if (version_ == 3) {
        std::string serialized;
        serialized.resize(sizeof(BinaryProtocol3) + packet->payload.size());
        auto bp3 = (BinaryProtocol3*)serialized.data();
        bp3->type = 0;
        bp3->reserved = 0;
        bp3->payload_size = htons(packet->payload.size());
        memcpy(bp3->payload, packet->payload.data(), packet->payload.size());

        return websocket_->Send(serialized.data(), serialized.size(), true);
    } else {
        return websocket_->Send(packet->payload.data(), packet->payload.size(), true);
    }
}

bool WebsocketProtocol::SendText(const std::string& text) {
    if (websocket_ == nullptr || !websocket_->IsConnected()) {
        return false;
    }

    if (!websocket_->Send(text)) {
        ESP_LOGE(TAG, "Failed to send text: %s", text.c_str());
        SetError(Lang::Strings::SERVER_ERROR);
        return false;
    }

    return true;
}

bool WebsocketProtocol::IsAudioChannelOpened() const {
    return websocket_ != nullptr && websocket_->IsConnected() && !error_occurred_ && !IsTimeout();
}

void WebsocketProtocol::CloseAudioChannel() {
    websocket_.reset();
}

bool WebsocketProtocol::OpenAudioChannel() {
    // 创建WebSocket实例
    auto network = Board::GetInstance().GetNetwork();
    if (network == nullptr) {
        ESP_LOGE(TAG, "Network interface is null");
        SetError(Lang::Strings::SERVER_NOT_CONNECTED);
        return false;
    }
    
    websocket_ = network->CreateWebSocket(0);
    if (websocket_ == nullptr) {
        ESP_LOGE(TAG, "Failed to create WebSocket instance");
        SetError(Lang::Strings::SERVER_NOT_CONNECTED);
        return false;
    }
    
    // 设置回调函数
    websocket_->OnData([this](const char* data, size_t len, bool binary) {
        if (binary) {
            // 处理音频数据
            if (version_ == 2) {
                if (len < sizeof(BinaryProtocol2)) return;
                auto bp2 = (BinaryProtocol2*)data;
                auto packet = std::make_unique<AudioStreamPacket>();
                packet->sample_rate = server_sample_rate_;  // 添加这行
                packet->frame_duration = server_frame_duration_;  // 添加这行
                packet->timestamp = ntohl(bp2->timestamp);
                packet->payload.assign(bp2->payload, bp2->payload + ntohl(bp2->payload_size));
                if (on_incoming_audio_ != nullptr) {
                    on_incoming_audio_(std::move(packet));
                }
            } else if (version_ == 3) {
                if (len < sizeof(BinaryProtocol3)) return;
                auto bp3 = (BinaryProtocol3*)data;
                auto packet = std::make_unique<AudioStreamPacket>();
                packet->sample_rate = server_sample_rate_;  // 添加这行
                packet->frame_duration = server_frame_duration_;  // 添加这行
                packet->payload.assign(bp3->payload, bp3->payload + ntohs(bp3->payload_size));
                if (on_incoming_audio_ != nullptr) {
                    on_incoming_audio_(std::move(packet));
                }
            } else {
                // 版本1：直接是音频数据
                auto packet = std::make_unique<AudioStreamPacket>();
                packet->sample_rate = server_sample_rate_;  // 添加这行
                packet->frame_duration = server_frame_duration_;  // 添加这行
                packet->payload.assign(data, data + len);
                if (on_incoming_audio_ != nullptr) {
                    on_incoming_audio_(std::move(packet));
                }
            }  // 添加这个缺失的右大括号
        } else {
            // 处理JSON消息
            std::string json_data(data, len);
            ESP_LOGI(TAG, "=== Received JSON Message ===");
            // 改为更兼容的长度输出（避免 %zu 在某些配置下不生效）
            ESP_LOGI(TAG, "Message length: %d bytes", (int)len);
            
            cJSON* root = cJSON_Parse(json_data.c_str());
            if (root == nullptr) {
                ESP_LOGE(TAG, "Failed to parse json message %s", json_data.c_str());
                return;
            }
            
            // 新增：格式化打印 JSON
            char* formatted_json = cJSON_Print(root);
            if (formatted_json != nullptr) {
                ESP_LOGI(TAG, "Received JSON (formatted):\n%s", formatted_json);
                free(formatted_json);
            } else {
            // 回退：打印原始 JSON
            // ESP_LOGI(TAG, "Raw JSON data: %s", json_data.c_str());
            }
            
            cJSON* type = cJSON_GetObjectItem(root, "type");
            if (!cJSON_IsString(type)) {
                ESP_LOGE(TAG, "Message type is invalid");
                cJSON_Delete(root);
                return;
            }
            
            ESP_LOGI(TAG, "Message type: %s", type->valuestring);
            
            // 如果是TTS消息，打印详细信息
            if (strcmp(type->valuestring, "tts") == 0) {
                ESP_LOGI(TAG, "=== TTS Message Details ===");
                
                cJSON* session_id = cJSON_GetObjectItem(root, "session_id");
                if (cJSON_IsString(session_id)) {
                    ESP_LOGI(TAG, "Session ID: %s", session_id->valuestring);
                }
                
                cJSON* state = cJSON_GetObjectItem(root, "state");
                if (cJSON_IsString(state)) {
                    ESP_LOGI(TAG, "TTS State: %s", state->valuestring);
                }
                
                cJSON* text = cJSON_GetObjectItem(root, "text");
                if (cJSON_IsString(text)) {
                    // ESP_LOGI(TAG, "TTS Text: %s", text->valuestring);
                }
                
                // 打印完整的格式化JSON
                char* formatted_json = cJSON_Print(root);
                if (formatted_json) {
                    // ESP_LOGI(TAG, "Formatted TTS JSON:\n%s", formatted_json);
                    free(formatted_json);
                }
                ESP_LOGI(TAG, "=== End TTS Message Details ===");
            }

            if (strcmp(type->valuestring, "hello") == 0) {
                ESP_LOGI(TAG, "Received server hello");
                ParseServerHello(root);
            } else if (on_incoming_json_ != nullptr) {
                on_incoming_json_(root);
            }
            cJSON_Delete(root);
            
            ESP_LOGI(TAG, "=== JSON Message Processing Complete ===");
        }
    });
    
    websocket_->OnDisconnected([this]() {
        ESP_LOGI(TAG, "WebSocket disconnected");
        if (on_audio_channel_closed_ != nullptr) {
            on_audio_channel_closed_();
        }
    });

    Settings settings("websocket", false);

    // 从配置中获取URL
    // 强制使用默认URL，忽略配置
    std::string url = settings.GetString("url");
    if (url.empty()) {
        std::string url = "wss://voice-proxy.derror.com/xiaozhi/v1/";
    }

    // Header 固定
    websocket_->SetHeader("Protocol-Version", "1");
    websocket_->SetHeader("Authorization", "Bearer test-token");
    websocket_->SetHeader("Device-Id", SystemInfo::GetMacAddress().c_str());
    websocket_->SetHeader("Client-Id", "caihong");

    //打印当前使用的url地址
    ESP_LOGI(TAG, "当前正在使用的服务器地址: %s", url.c_str());

    ESP_LOGI(TAG, "Connecting to websocket server: %s", url.c_str());
    if (!websocket_->Connect(url.c_str())) {
        ESP_LOGE(TAG, "WS 连接失败 Failed to connect to websocket server");
        SetError(Lang::Strings::SERVER_NOT_CONNECTED);
        return false;
    }

    // 所在方法：Start 或建立连接后发送 hello 的位置
    // 发送客户端Hello消息描述设备能力
    auto message = GetHelloMessage();
    
    // 新增：将 hello 的 JSON 美化后打印
    cJSON* hello_json = cJSON_Parse(message.c_str());
    if (hello_json != nullptr) {
        char* formatted_json = cJSON_Print(hello_json);
        if (formatted_json != nullptr) {
            ESP_LOGI(TAG, "Sending hello message (formatted):\n%s", formatted_json);
            free(formatted_json);
        } else {
            ESP_LOGI(TAG, "Sending hello message: %s", message.c_str());
        }
        cJSON_Delete(hello_json);
    } else {
        ESP_LOGI(TAG, "Sending hello message: %s", message.c_str());
    }

    if (!SendText(message)) {
        ESP_LOGE(TAG, "Failed to send hello message");
        return false;
    }

    // 等待服务器Hello响应（超时10秒）
    EventBits_t bits = xEventGroupWaitBits(event_group_handle_, WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT, pdTRUE, pdFALSE, pdMS_TO_TICKS(10000));
    if (!(bits & WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT)) {
        ESP_LOGE(TAG, "Failed to receive server hello");
        SetError(Lang::Strings::SERVER_TIMEOUT);
        return false;
    }

    // 触发音频通道打开回调
    if (on_audio_channel_opened_ != nullptr) {
        on_audio_channel_opened_();
    }

    return true;
}

std::string WebsocketProtocol::GetHelloMessage() {
    // keys: message type, version, audio_params (format, sample_rate, channels)
    cJSON* root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "type", "hello");
    cJSON_AddNumberToObject(root, "version", version_);
    cJSON* features = cJSON_CreateObject();
#if CONFIG_USE_SERVER_AEC
    cJSON_AddBoolToObject(features, "aec", true);
#endif
    cJSON_AddBoolToObject(features, "mcp", true);
    cJSON_AddItemToObject(root, "features", features);
    cJSON_AddStringToObject(root, "transport", "websocket");
    cJSON* audio_params = cJSON_CreateObject();
    cJSON_AddStringToObject(audio_params, "format", "opus");
    cJSON_AddNumberToObject(audio_params, "sample_rate", 16000);
    cJSON_AddNumberToObject(audio_params, "channels", 1);
    cJSON_AddNumberToObject(audio_params, "frame_duration", OPUS_FRAME_DURATION_MS);
    cJSON_AddItemToObject(root, "audio_params", audio_params);
    auto json_str = cJSON_PrintUnformatted(root);
    std::string message(json_str);
    cJSON_free(json_str);
    cJSON_Delete(root);
    return message;
}

void WebsocketProtocol::ParseServerHello(const cJSON* root) {
    auto transport = cJSON_GetObjectItem(root, "transport");
    if (transport == nullptr || strcmp(transport->valuestring, "websocket") != 0) {
        ESP_LOGE(TAG, "Unsupported transport: %s", transport->valuestring);
        return;
    }

    auto session_id = cJSON_GetObjectItem(root, "session_id");
    if (cJSON_IsString(session_id)) {
        session_id_ = session_id->valuestring;
        ESP_LOGI(TAG, "Session ID: %s", session_id_.c_str());
    }

    auto audio_params = cJSON_GetObjectItem(root, "audio_params");
    if (cJSON_IsObject(audio_params)) {
        auto sample_rate = cJSON_GetObjectItem(audio_params, "sample_rate");
        if (cJSON_IsNumber(sample_rate)) {
            server_sample_rate_ = sample_rate->valueint;
        }
        auto frame_duration = cJSON_GetObjectItem(audio_params, "frame_duration");
        if (cJSON_IsNumber(frame_duration)) {
            server_frame_duration_ = frame_duration->valueint;
        }
    }

    xEventGroupSetBits(event_group_handle_, WEBSOCKET_PROTOCOL_SERVER_HELLO_EVENT);
}
