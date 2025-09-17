#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  16000          // ���������
#define AUDIO_OUTPUT_SAMPLE_RATE 16000          // ���������

// ��Ƶ����ο�����
#define AUDIO_INPUT_REFERENCE    true          //�Ƿ�������Ƶ����ο�������������

// I2S ��Ƶ���ݴ������ţ�����ԭ��ͼ1018_MCK�ȱ�ע��
#define AUDIO_I2S_GPIO_MCLK      GPIO_NUM_18       // ��ʱ������ (1018_MCK)
#define AUDIO_I2S_GPIO_WS        GPIO_NUM_15       // ��ʱ��/����ʱ�� (1015_LCK)
#define AUDIO_I2S_GPIO_BCLK      GPIO_NUM_17       // λʱ�� (1017_SCK) 
#define AUDIO_I2S_GPIO_DIN       GPIO_NUM_16       // �������� (1014_DS0IN)
#define AUDIO_I2S_GPIO_DOUT      GPIO_NUM_14       // ������� (1016_ASOOUT)

// I2C ���ƽӿ�����
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_48        // I2C ���� (1048_SDA)
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_47        // I2C ʱ�� (1047_CLK)
#define AUDIO_CODEC_ES8311_ADDR  0x30               // ES8311Ĭ�ϵ�ַ

// ������������
#define AUDIO_CODEC_PA_PIN       GPIO_NUM_21        // ES8311�Ĺ���ʹ�ܿ�������

#define BOOT_BUTTON_GPIO         GPIO_NUM_0         // ������ť����
#define KEY_BUTTON_GPIO          GPIO_NUM_3         // ������ť����
#define LED_PIN                  GPIO_NUM_11        // ״ָ̬ʾ������

#define VBAT_ADC_CHANNEL         ADC_CHANNEL_1      // ��ص�ѹADCͨ�� (GPIO2)
#define CHARGE_DETECT_GPIO       GPIO_NUM_1         // ��������� (GPIO1)
#define MCU_VCC_CTL              GPIO_NUM_10        // MCU�����������
#define PERP_VCC_CTL             GPIO_NUM_6         // ���蹩���������

#define ADC_ATTEN                ADC_ATTEN_DB_12          // ADC ˥�� ���� ADC �����ѹ��Χ ˥�� 12dB��������Χ ??0~3.3V
#define ADC_WIDTH                ADC_BITWIDTH_DEFAULT     // ADC λ�� ���� ADC ����λ��Ĭ��λ��ESP32 ͨ���� 12 λ��
#define FULL_BATTERY_VOLTAGE     4100               // �����ѹ       
#define EMPTY_BATTERY_VOLTAGE    3400               // û���ѹ

#endif // _BOARD_CONFIG_H_
