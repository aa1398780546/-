#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  16000          // 输入采样率
#define AUDIO_OUTPUT_SAMPLE_RATE 16000          // 输出采样率

// 音频输入参考配置
#define AUDIO_INPUT_REFERENCE    true          //是否启用音频输入参考（回声消除）

// I2S 音频数据传输引脚（根据原理图1018_MCK等标注）
#define AUDIO_I2S_GPIO_MCLK      GPIO_NUM_18       // 主时钟引脚 (1018_MCK)
#define AUDIO_I2S_GPIO_WS        GPIO_NUM_15       // 字时钟/左右时钟 (1015_LCK)
#define AUDIO_I2S_GPIO_BCLK      GPIO_NUM_17       // 位时钟 (1017_SCK) 
#define AUDIO_I2S_GPIO_DIN       GPIO_NUM_16       // 数据输入 (1014_DS0IN)
#define AUDIO_I2S_GPIO_DOUT      GPIO_NUM_14       // 数据输出 (1016_ASOOUT)

// I2C 控制接口引脚
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_48        // I2C 数据 (1048_SDA)
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_47        // I2C 时钟 (1047_CLK)
#define AUDIO_CODEC_ES8311_ADDR  0x30               // ES8311默认地址

// 其他控制引脚
#define AUDIO_CODEC_PA_PIN       GPIO_NUM_21        // ES8311的功放使能控制引脚

#define BOOT_BUTTON_GPIO         GPIO_NUM_0         // 启动按钮引脚
#define KEY_BUTTON_GPIO          GPIO_NUM_3         // 按键按钮引脚
#define LED_PIN                  GPIO_NUM_11        // 状态指示灯引脚

#define VBAT_ADC_CHANNEL         ADC_CHANNEL_1      // 电池电压ADC通道 (GPIO2)
#define CHARGE_DETECT_GPIO       GPIO_NUM_1         // 充电检测引脚 (GPIO1)
#define MCU_VCC_CTL              GPIO_NUM_10        // MCU供电控制引脚
#define PERP_VCC_CTL             GPIO_NUM_6         // 外设供电控制引脚

#define ADC_ATTEN                ADC_ATTEN_DB_12          // ADC 衰减 设置 ADC 输入电压范围 衰减 12dB，测量范围 ??0~3.3V
#define ADC_WIDTH                ADC_BITWIDTH_DEFAULT     // ADC 位宽 设置 ADC 采样位数默认位宽（ESP32 通常是 12 位）
#define FULL_BATTERY_VOLTAGE     4100               // 满电电压       
#define EMPTY_BATTERY_VOLTAGE    3400               // 没电电压

#endif // _BOARD_CONFIG_H_
