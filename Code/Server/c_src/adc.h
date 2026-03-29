/*
 * adc.h — ADS7830 ADC芯片 I2C驱动
 *
 * 对应Python: adc.py (class ADC)
 * 硬件: ADS7830，8通道8位ADC，I2C地址 0x48
 *
 * 通道用途:
 *   通道0 — 左侧光敏电阻（追光模式用）
 *   通道1 — 右侧光敏电阻（追光模式用）
 *   通道2 — 电池电压检测
 *
 * 电压系数（由PCB版本决定）:
 *   PCB v1: 系数 3.3  → 电压 = raw / 255.0 * 3.3
 *   PCB v2: 系数 5.2  → 电压 = raw / 255.0 * 5.2
 */

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* ── ADS7830 配置 ────────────────────────────────────── */
#define ADC_I2C_ADDRESS   0x48
#define ADC_COMMAND_BASE  0x84  /* 基础命令字节 */

typedef struct {
    int   fd;               /* I2C 文件描述符 */
    int   pcb_version;      /* 1 或 2，决定电压系数 */
    float voltage_coeff;    /* 3.3f (v1) 或 5.2f (v2) */
} ADC;

/**
 * adc_init - 初始化ADC，打开I2C总线，读取PCB版本
 * @adc: ADC句柄指针
 * 返回: 0=成功, -1=失败
 * 对应Python: ADC.__init__()
 */
int adc_init(ADC *adc);

/**
 * adc_read - 读取指定通道的电压值
 * @adc:     ADC句柄
 * @channel: 通道号（0~7）
 * 返回: 电压值（V），保留2位小数；失败返回 -1.0f
 * 对应Python: read_adc(channel)
 */
float adc_read(ADC *adc, int channel);

/**
 * adc_close - 关闭I2C文件描述符
 * @adc: ADC句柄
 * 对应Python: close_i2c()
 */
void adc_close(ADC *adc);

#endif /* ADC_H */
