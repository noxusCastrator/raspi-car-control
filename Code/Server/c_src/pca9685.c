/*
 * pca9685.c — PCA9685 16通道PWM控制器 I2C驱动实现
 *
 * 对应Python: pca9685.py (class PCA9685)
 *
 * I2C通信方式:
 *   使用Linux标准 /dev/i2c-N + ioctl(I2C_SLAVE) 接口
 *   无需第三方库，仅依赖内核头文件 <linux/i2c-dev.h>
 *
 * 寄存器写入协议 (write_byte_data):
 *   START + ADDR_W + REG + VALUE + STOP
 *   → write(fd, {reg, value}, 2)
 *
 * 寄存器读取协议 (read_byte_data):
 *   START + ADDR_W + REG + STOP + START + ADDR_R + VALUE + STOP
 *   → write(fd, &reg, 1)  +  read(fd, &value, 1)
 */

#include "pca9685.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <math.h>
#include <string.h>
#include <errno.h>

/* I2C_SLAVE ioctl请求码（来自 linux/i2c-dev.h）*/
#ifndef I2C_SLAVE
#define I2C_SLAVE 0x0703
#endif

/* ── 内部工具宏 ─────────────────────────────────────── */
#define I2C_DEV_PATH_FMT  "/dev/i2c-%d"
#define I2C_DEV_PATH_LEN  16

/* ── 初始化 ──────────────────────────────────────────── */

int pca9685_init(PCA9685 *dev, int address, int debug)
{
    char path[I2C_DEV_PATH_LEN];
    snprintf(path, sizeof(path), I2C_DEV_PATH_FMT, PCA9685_I2C_BUS);

    dev->address = address;
    dev->debug   = debug;
    dev->fd      = -1;

    /* 打开I2C总线设备文件 */
    dev->fd = open(path, O_RDWR);
    if (dev->fd < 0) {
        fprintf(stderr, "[PCA9685] 无法打开 %s: %s\n", path, strerror(errno));
        return -1;
    }

    /* 设置I2C从机地址 */
    if (ioctl(dev->fd, I2C_SLAVE, address) < 0) {
        fprintf(stderr, "[PCA9685] 设置I2C地址 0x%02X 失败: %s\n",
                address, strerror(errno));
        close(dev->fd);
        dev->fd = -1;
        return -1;
    }

    /* 复位：写MODE1=0x00，禁用省电/重启模式，启用内部振荡器 */
    pca9685_write(dev, PCA9685_MODE1, 0x00);

    if (dev->debug) {
        printf("[PCA9685] 初始化完成，I2C地址=0x%02X，总线=%s\n",
               address, path);
    }
    return 0;
}

/* ── 寄存器读写 ──────────────────────────────────────── */

void pca9685_write(PCA9685 *dev, uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    if (write(dev->fd, buf, 2) != 2) {
        if (dev->debug) {
            fprintf(stderr, "[PCA9685] 写寄存器 0x%02X 失败: %s\n",
                    reg, strerror(errno));
        }
    }
}

uint8_t pca9685_read(PCA9685 *dev, uint8_t reg)
{
    /* 先写寄存器地址，再读取数据 */
    if (write(dev->fd, &reg, 1) != 1) {
        if (dev->debug) {
            fprintf(stderr, "[PCA9685] 读取寄存器地址写入失败: %s\n",
                    strerror(errno));
        }
        return 0;
    }

    uint8_t value = 0;
    if (read(dev->fd, &value, 1) != 1) {
        if (dev->debug) {
            fprintf(stderr, "[PCA9685] 读寄存器 0x%02X 失败: %s\n",
                    reg, strerror(errno));
        }
        return 0;
    }
    return value;
}

/* ── PWM频率设置 ─────────────────────────────────────── */

/*
 * PCA9685频率计算公式（来自数据手册）:
 *   prescale = round(osc_clock / (4096 * freq)) - 1
 *             = round(25MHz / (4096 * freq)) - 1
 *
 * 步骤:
 *   1. 读取MODE1当前值
 *   2. 进入睡眠模式（bit4=1），停止振荡器
 *   3. 写入PRESCALE
 *   4. 恢复MODE1原值（退出睡眠）
 *   5. 等待振荡器稳定 (>500us，这里等5ms)
 *   6. 写MODE1 | 0x80，使能重启
 *
 * 对应Python: set_pwm_freq(freq)
 */
void pca9685_set_pwm_freq(PCA9685 *dev, float freq)
{
    double prescaleval = PCA9685_OSC_FREQ / 4096.0 / (double)freq - 1.0;
    int    prescale    = (int)floor(prescaleval + 0.5);

    uint8_t oldmode = pca9685_read(dev, PCA9685_MODE1);
    uint8_t newmode = (oldmode & 0x7F) | 0x10;   /* 设置睡眠位(bit4) */

    pca9685_write(dev, PCA9685_MODE1, newmode);   /* 进入睡眠 */
    pca9685_write(dev, PCA9685_PRESCALE, (uint8_t)prescale);
    pca9685_write(dev, PCA9685_MODE1, oldmode);   /* 退出睡眠 */
    usleep(5000);                                 /* 等待振荡器稳定 5ms */
    pca9685_write(dev, PCA9685_MODE1, oldmode | 0x80); /* 使能重启 */

    if (dev->debug) {
        printf("[PCA9685] PWM频率=%.1fHz，PRESCALE=0x%02X\n",
               freq, prescale);
    }
}

/* ── PWM通道控制 ─────────────────────────────────────── */

/*
 * 每个通道占4个连续寄存器（步长4）:
 *   LED_ON_L  = LED0_ON_L  + 4*channel
 *   LED_ON_H  = LED0_ON_H  + 4*channel
 *   LED_OFF_L = LED0_OFF_L + 4*channel
 *   LED_OFF_H = LED0_OFF_H + 4*channel
 *
 * on/off均为12位值（0~4095）
 * 对应Python: set_pwm(channel, on, off)
 */
void pca9685_set_pwm(PCA9685 *dev, int channel, int on, int off)
{
    uint8_t base = (uint8_t)(PCA9685_LED0_ON_L + 4 * channel);
    pca9685_write(dev, base,     (uint8_t)(on  & 0xFF));
    pca9685_write(dev, base + 1, (uint8_t)(on  >> 8));
    pca9685_write(dev, base + 2, (uint8_t)(off & 0xFF));
    pca9685_write(dev, base + 3, (uint8_t)(off >> 8));
}

/*
 * 电机PWM：on固定为0，直接设置占空比
 * 对应Python: set_motor_pwm(channel, duty)
 */
void pca9685_set_motor_pwm(PCA9685 *dev, int channel, int duty)
{
    pca9685_set_pwm(dev, channel, 0, duty);
}

/*
 * 舵机脉宽：将微秒转换为12位计数值
 *   50Hz周期 = 20000us，对应4096个计数
 *   count = pulse_us * 4096 / 20000
 * 对应Python: set_servo_pulse(channel, pulse)
 */
void pca9685_set_servo_pulse(PCA9685 *dev, int channel, float pulse_us)
{
    int count = (int)(pulse_us * 4096.0f / 20000.0f);
    pca9685_set_pwm(dev, channel, 0, count);
}

/* ── 关闭 ────────────────────────────────────────────── */

void pca9685_close(PCA9685 *dev)
{
    if (dev->fd >= 0) {
        close(dev->fd);
        dev->fd = -1;
        if (dev->debug) {
            printf("[PCA9685] I2C总线已关闭\n");
        }
    }
}
