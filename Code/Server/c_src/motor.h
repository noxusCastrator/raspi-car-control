/*
 * motor.h — 四轮电机控制驱动
 *
 * 对应Python: motor.py (class Ordinary_Car)
 * 依赖: pca9685（通过I2C控制电机驱动板）
 *
 * PCA9685通道映射（与Python一致）:
 *   左前轮 (FL): ch0(正转) / ch1(反转)
 *   左后轮 (BL): ch2(正转) / ch3(反转)
 *   右前轮 (FR): ch6(正转) / ch7(反转)
 *   右后轮 (BR): ch4(正转) / ch5(反转)
 *
 * 占空比约定:
 *   正值 → 正转，负值 → 反转，0 → 刹车（两路均拉高至4095）
 *   范围限制: [-4095, +4095]
 */

#ifndef MOTOR_H
#define MOTOR_H

#include "pca9685.h"

/* ── 电机占空比限制 ───────────────────────────────────── */
#define MOTOR_DUTY_MAX   4095
#define MOTOR_DUTY_MIN  -4095
#define MOTOR_BRAKE_VAL  4095   /* 刹车时两路均置此值 */

/* ── 设备句柄 ────────────────────────────────────────── */
typedef struct {
    PCA9685 pwm;    /* 复用PCA9685实例 */
} Motor;

/* ── 公开API ─────────────────────────────────────────── */

/**
 * motor_init - 初始化电机控制器，设置PWM频率为50Hz
 * @motor: 电机句柄指针
 * 返回: 0=成功, -1=失败
 * 对应Python: Ordinary_Car.__init__()
 */
int motor_init(Motor *motor);

/**
 * motor_duty_clamp - 将占空比截断到 [-4095, +4095]
 * @duty: 输入占空比
 * 返回: 截断后的占空比
 * 对应Python: duty_range()
 */
int motor_duty_clamp(int duty);

/**
 * motor_set_model - 同时设置四轮速度（核心接口）
 * @motor:   电机句柄
 * @fl: 左前轮占空比   @bl: 左后轮占空比
 * @fr: 右前轮占空比   @br: 右后轮占空比
 *
 * 常用组合:
 *   前进: motor_set_model(m,  2000,  2000,  2000,  2000)
 *   后退: motor_set_model(m, -2000, -2000, -2000, -2000)
 *   左转: motor_set_model(m, -2000, -2000,  2000,  2000)
 *   右转: motor_set_model(m,  2000,  2000, -2000, -2000)
 *   停止: motor_set_model(m,  0,     0,     0,     0)
 *
 * 对应Python: set_motor_model(duty1, duty2, duty3, duty4)
 */
void motor_set_model(Motor *motor, int fl, int bl, int fr, int br);

/**
 * motor_close - 停止所有电机并释放PCA9685资源
 * @motor: 电机句柄
 * 对应Python: close()
 */
void motor_close(Motor *motor);

#endif /* MOTOR_H */
