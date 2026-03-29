/*
 * adc.c — ADS7830 ADC芯片 I2C驱动实现
 *
 * 对应Python: adc.py (class ADC)
 *
 * ADS7830 通道选择命令字节构造（来自Python原版）:
 *   command = 0x84 | ((((ch << 2) | (ch >> 1)) & 0x07) << 4)
 *
 *   通道0: 0x84, 通道1: 0xC4, 通道2: 0x94
 *   通道3: 0xD4, 通道4: 0xA4, 通道5: 0xE4
 *   通道6: 0xB4, 通道7: 0xF4
 *
 * 稳定读取: 连续两次读到相同值才返回（过滤ADC抖动）
 * 对应Python: _read_stable_byte()
 */

#include "adc.h"
#include "parameter.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#ifndef I2C_SLAVE
#define I2C_SLAVE 0x0703
#endif

#define ADC_I2C_BUS  1   /* /dev/i2c-1 */

/* ── 初始化 ──────────────────────────────────────────── */

int adc_init(ADC *adc)
{
    char path[16];
    snprintf(path, sizeof(path), "/dev/i2c-%d", ADC_I2C_BUS);

    adc->fd = open(path, O_RDWR);
    if (adc->fd < 0) {
        fprintf(stderr, "[ADC] 无法打开 %s: %s\n", path, strerror(errno));
        return -1;
    }

    if (ioctl(adc->fd, I2C_SLAVE, ADC_I2C_ADDRESS) < 0) {
        fprintf(stderr, "[ADC] 设置I2C地址 0x%02X 失败: %s\n",
                ADC_I2C_ADDRESS, strerror(errno));
        close(adc->fd);
        adc->fd = -1;
        return -1;
    }

    /* 读取PCB版本，确定电压系数 */
    adc->pcb_version    = param_get_pcb_version();
    adc->voltage_coeff  = (adc->pcb_version == 1) ? 3.3f : 5.2f;

    printf("[ADC] 初始化完成，PCB版本=%d，电压系数=%.1fV\n",
           adc->pcb_version, adc->voltage_coeff);
    return 0;
}

/* ── 内部：稳定读取 ──────────────────────────────────── */

/*
 * _read_stable_byte - 连续两次读到相同值才返回
 * 对应Python: _read_stable_byte()
 *
 * 最多重试 10 次防止死循环（原Python无上限，实际ADC很快稳定）
 */
static int _read_stable_byte(int fd)
{
    uint8_t v1, v2;
    int retries = 10;

    do {
        if (read(fd, &v1, 1) != 1) return -1;
        if (read(fd, &v2, 1) != 1) return -1;
        retries--;
    } while (v1 != v2 && retries > 0);

    return (int)v1;
}

/* ── 读取通道电压 ────────────────────────────────────── */

/*
 * adc_read - 读取指定通道的电压（V）
 *
 * ADS7830 通道选择位构造:
 *   channel_bits = ((ch << 2) | (ch >> 1)) & 0x07
 *   command = ADC_COMMAND_BASE | (channel_bits << 4)
 *
 * 写入命令后立即读取转换结果（ADS7830 单次转换模式）
 * 对应Python: read_adc(channel)
 */
float adc_read(ADC *adc, int channel)
{
    if (channel < 0 || channel > 7) {
        fprintf(stderr, "[ADC] 无效通道: %d\n", channel);
        return -1.0f;
    }

    /* 构造通道选择命令 */
    uint8_t channel_bits = (uint8_t)(((channel << 2) | (channel >> 1)) & 0x07);
    uint8_t command = (uint8_t)(ADC_COMMAND_BASE | (channel_bits << 4));

    /* 发送命令 */
    if (write(adc->fd, &command, 1) != 1) {
        fprintf(stderr, "[ADC] 写命令失败: %s\n", strerror(errno));
        return -1.0f;
    }

    /* 稳定读取原始值 */
    int raw = _read_stable_byte(adc->fd);
    if (raw < 0) {
        fprintf(stderr, "[ADC] 读取失败: %s\n", strerror(errno));
        return -1.0f;
    }

    /* 转换为电压，保留2位小数 */
    float voltage = (float)raw / 255.0f * adc->voltage_coeff;
    voltage = roundf(voltage * 100.0f) / 100.0f;
    return voltage;
}

/* ── 关闭 ────────────────────────────────────────────── */

void adc_close(ADC *adc)
{
    if (adc->fd >= 0) {
        close(adc->fd);
        adc->fd = -1;
        printf("[ADC] I2C总线已关闭\n");
    }
}
