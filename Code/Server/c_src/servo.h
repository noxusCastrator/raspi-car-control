/*
 * servo.h — 舵机控制驱动
 *
 * 对应Python: servo.py (class Servo)
 * 依赖: pca9685（复用电机的同一芯片，不同通道）
 *
 * 逻辑通道 → PCA9685物理通道映射:
 *   逻辑 0 → 物理 ch8    逻辑 4 → 物理 ch12
 *   逻辑 1 → 物理 ch9    逻辑 5 → 物理 ch13
 *   逻辑 2 → 物理 ch10   逻辑 6 → 物理 ch14
 *   逻辑 3 → 物理 ch11   逻辑 7 → 物理 ch15
 *
 * 脉宽计算（频率50Hz，周期20000us）:
 *   逻辑通道0（特殊，反向）: pulse = 2500 - (angle + error) / 0.09
 *   其他通道:               pulse = 500  + (angle + error) / 0.09
 *
 * 初始脉宽: 1500us（所有通道归中位）
 */

#ifndef SERVO_H
#define SERVO_H

#include "pca9685.h"

/* ── 舵机参数 ────────────────────────────────────────── */
#define SERVO_CHANNEL_COUNT  8      /* 逻辑通道数 */
#define SERVO_FREQ_HZ        50     /* PWM频率 */
#define SERVO_INIT_PULSE_US  1500   /* 初始脉宽（归中）*/
#define SERVO_DEFAULT_ERROR  10     /* 默认误差补偿（来自Python原版）*/

typedef struct {
    PCA9685 pwm;
} Servo;

/**
 * servo_init - 初始化舵机控制器，所有通道归中位
 * @servo: 舵机句柄指针
 * 返回: 0=成功, -1=失败
 * 对应Python: Servo.__init__()
 */
int servo_init(Servo *servo);

/**
 * servo_set_angle - 设置指定逻辑通道的角度
 * @servo:   舵机句柄
 * @channel: 逻辑通道号（0~7）
 * @angle:   目标角度（0~180度）
 * @error:   误差补偿（通常传 SERVO_DEFAULT_ERROR）
 * 对应Python: set_servo_pwm(channel, angle, error)
 */
void servo_set_angle(Servo *servo, int channel, int angle, int error);

/**
 * servo_close - 不做特殊处理（PCA9685由调用方负责关闭）
 * 保留以维持接口对称性
 */
void servo_close(Servo *servo);

#endif /* SERVO_H */
