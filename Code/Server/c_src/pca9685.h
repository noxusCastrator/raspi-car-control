/*
 * pca9685.h — PCA9685 16通道PWM控制器 I2C驱动
 *
 * 对应Python: pca9685.py (class PCA9685)
 * 硬件接口: I2C总线 /dev/i2c-1，默认地址 0x40
 *
 * PCA9685 寄存器地址参考:
 *   MODE1     0x00 — 工作模式寄存器
 *   PRESCALE  0xFE — PWM频率预分频寄存器
 *   LED0_ON_L 0x06 — 通道0 ON低位（每通道占4个寄存器，步长4）
 */

#ifndef PCA9685_H
#define PCA9685_H

#include <stdint.h>

/* ── 寄存器地址 ─────────────────────────────────────── */
#define PCA9685_MODE1       0x00
#define PCA9685_PRESCALE    0xFE
#define PCA9685_LED0_ON_L   0x06
#define PCA9685_LED0_ON_H   0x07
#define PCA9685_LED0_OFF_L  0x08
#define PCA9685_LED0_OFF_H  0x09

/* ── 默认参数 ────────────────────────────────────────── */
#define PCA9685_DEFAULT_ADDR    0x40    /* 默认I2C地址 */
#define PCA9685_I2C_BUS         1       /* /dev/i2c-1  */
#define PCA9685_OSC_FREQ        25000000.0  /* 片内振荡器 25MHz */

/* ── 设备句柄 ────────────────────────────────────────── */
typedef struct {
    int     fd;         /* I2C文件描述符 */
    int     address;    /* I2C从机地址   */
    int     debug;      /* 调试输出开关  */
} PCA9685;

/* ── 公开API ─────────────────────────────────────────── */

/**
 * pca9685_init - 初始化PCA9685，打开I2C总线并复位芯片
 * @dev:     设备句柄指针
 * @address: I2C地址（通常 0x40）
 * @debug:   1=打印调试信息，0=静默
 * 返回: 0=成功, -1=失败
 */
int pca9685_init(PCA9685 *dev, int address, int debug);

/**
 * pca9685_write - 向指定寄存器写入1字节
 * @dev: 设备句柄
 * @reg: 寄存器地址
 * @value: 待写入值
 */
void pca9685_write(PCA9685 *dev, uint8_t reg, uint8_t value);

/**
 * pca9685_read - 从指定寄存器读取1字节
 * @dev: 设备句柄
 * @reg: 寄存器地址
 * 返回: 读取的字节值
 */
uint8_t pca9685_read(PCA9685 *dev, uint8_t reg);

/**
 * pca9685_set_pwm_freq - 设置所有通道的PWM频率
 * @dev:  设备句柄
 * @freq: 目标频率（Hz），电机/舵机通常使用50Hz
 * 对应Python: set_pwm_freq(freq)
 */
void pca9685_set_pwm_freq(PCA9685 *dev, float freq);

/**
 * pca9685_set_pwm - 设置单通道PWM的ON/OFF计数值（0~4095）
 * @dev:     设备句柄
 * @channel: 通道号（0~15）
 * @on:      PWM上升沿计数（通常为0）
 * @off:     PWM下降沿计数（即占空比，0~4095）
 * 对应Python: set_pwm(channel, on, off)
 */
void pca9685_set_pwm(PCA9685 *dev, int channel, int on, int off);

/**
 * pca9685_set_motor_pwm - 电机专用：设置占空比（on固定为0）
 * @dev:     设备句柄
 * @channel: 通道号
 * @duty:    占空比（0~4095）
 * 对应Python: set_motor_pwm(channel, duty)
 */
void pca9685_set_motor_pwm(PCA9685 *dev, int channel, int duty);

/**
 * pca9685_set_servo_pulse - 舵机专用：设置脉宽（微秒）
 * @dev:      设备句柄
 * @channel:  通道号（8~15为舵机通道）
 * @pulse_us: 脉宽，单位微秒（500~2500us）
 * 对应Python: set_servo_pulse(channel, pulse)
 */
void pca9685_set_servo_pulse(PCA9685 *dev, int channel, float pulse_us);

/**
 * pca9685_close - 关闭I2C文件描述符，释放资源
 * @dev: 设备句柄
 */
void pca9685_close(PCA9685 *dev);

#endif /* PCA9685_H */
