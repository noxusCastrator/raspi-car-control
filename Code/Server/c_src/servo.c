/*
 * servo.c — 舵机控制驱动实现
 *
 * 对应Python: servo.py (class Servo)
 *
 * 脉宽→角度换算（来自Python原版）:
 *   逻辑通道0（反向）: pulse_us = 2500 - (int)((angle + error) / 0.09)
 *   其他通道（正向）:  pulse_us = 500  + (int)((angle + error) / 0.09)
 *
 * 初始化时向所有通道发送 1500us（90度，中位）
 *
 * 注意：Servo 与 Motor 共用同一 PCA9685 芯片（地址0x40），
 * 但使用不同通道（舵机用 ch8~ch15，电机用 ch0~ch7），
 * 两者各自持有独立的I2C文件描述符，在Linux内核层面串行化，互不干扰。
 */

#include "servo.h"
#include <stdio.h>

/* ── 逻辑通道 → 物理通道映射 ────────────────────────── */
/* 对应Python: pwm_channel_map = {'0':8, '1':9, ..., '7':15} */
static const int CHANNEL_MAP[SERVO_CHANNEL_COUNT] = {8, 9, 10, 11, 12, 13, 14, 15};

/* ── 初始化 ──────────────────────────────────────────── */

int servo_init(Servo *servo)
{
    if (pca9685_init(&servo->pwm, PCA9685_DEFAULT_ADDR, 1) < 0) {
        fprintf(stderr, "[Servo] PCA9685初始化失败\n");
        return -1;
    }
    pca9685_set_pwm_freq(&servo->pwm, SERVO_FREQ_HZ);

    /* 所有通道归中位（1500us）*/
    for (int i = 0; i < SERVO_CHANNEL_COUNT; i++) {
        pca9685_set_servo_pulse(&servo->pwm, CHANNEL_MAP[i], SERVO_INIT_PULSE_US);
    }

    printf("[Servo] 初始化完成，所有通道归中位（1500us）\n");
    return 0;
}

/* ── 角度设置 ────────────────────────────────────────── */

/*
 * servo_set_angle - 将角度转换为脉宽并发送给PCA9685
 *
 * 换算公式（对应Python set_servo_pwm）:
 *   通道0（ch8, 反向安装）:
 *     pulse = 2500 - (int)((angle + error) / 0.09)
 *   其他通道（正向）:
 *     pulse = 500 + (int)((angle + error) / 0.09)
 *
 * 角度范围: 0~180度
 * 脉宽范围: 500~2500us（50Hz/20000us周期）
 */
void servo_set_angle(Servo *servo, int channel, int angle, int error)
{
    if (channel < 0 || channel >= SERVO_CHANNEL_COUNT) {
        fprintf(stderr, "[Servo] 无效逻辑通道: %d（有效范围 0~%d）\n",
                channel, SERVO_CHANNEL_COUNT - 1);
        return;
    }

    int pulse_us;
    if (channel == 0) {
        /* 通道0安装方向相反，使用反向公式 */
        pulse_us = 2500 - (int)((angle + error) / 0.09);
    } else {
        pulse_us = 500 + (int)((angle + error) / 0.09);
    }

    int phys_channel = CHANNEL_MAP[channel];
    pca9685_set_servo_pulse(&servo->pwm, phys_channel, (float)pulse_us);
}

/* ── 关闭 ────────────────────────────────────────────── */

void servo_close(Servo *servo)
{
    pca9685_close(&servo->pwm);
    printf("[Servo] 已关闭\n");
}
