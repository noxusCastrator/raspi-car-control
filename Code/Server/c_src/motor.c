/*
 * motor.c — 四轮电机控制驱动实现
 *
 * 对应Python: motor.py (class Ordinary_Car)
 *
 * 电机驱动板通过PCA9685的PWM输出控制电机的H桥。
 * 每个轮子对应两个PCA9685通道（ch_a / ch_b），通过以下逻辑控制方向：
 *
 *   duty > 0 : ch_a = 0,      ch_b = duty  （正转）
 *   duty < 0 : ch_b = 0,      ch_a = |duty|（反转）
 *   duty = 0 : ch_a = 4095,   ch_b = 4095  （刹车，双路拉高）
 *
 * PCA9685通道分配（严格对应Python motor.py 各函数行为）:
 *
 *   左前轮 FL (left_upper_wheel):
 *     duty>0: ch0=0,    ch1=duty  → ch_a=0, ch_b=1
 *
 *   左后轮 BL (left_lower_wheel):
 *     duty>0: ch3=0,    ch2=duty  → ch_a=3, ch_b=2  ← 注意顺序!
 *
 *   右前轮 FR (right_upper_wheel):
 *     duty>0: ch6=0,    ch7=duty  → ch_a=6, ch_b=7
 *
 *   右后轮 BR (right_lower_wheel):
 *     duty>0: ch4=0,    ch5=duty  → ch_a=4, ch_b=5
 */

#include "motor.h"
#include <stdio.h>

/* ── 内部单轮控制函数 ────────────────────────────────── */

/*
 * _wheel_set - 设置单轮PWM（内部使用）
 *
 * @pwm:   PCA9685句柄
 * @ch_a:  duty>0时置0的通道（见各轮映射注释）
 * @ch_b:  duty>0时输出duty的通道
 * @duty:  占空比（已截断，范围 [-4095, 4095]）
 *
 * 行为严格对应Python各 *_wheel 函数:
 *   duty > 0: ch_a = 0,      ch_b = duty
 *   duty < 0: ch_b = 0,      ch_a = |duty|
 *   duty = 0: ch_a = 4095,   ch_b = 4095  (刹车)
 */
static void _wheel_set(PCA9685 *pwm, int ch_a, int ch_b, int duty)
{
    if (duty > 0) {
        pca9685_set_motor_pwm(pwm, ch_a, 0);
        pca9685_set_motor_pwm(pwm, ch_b, duty);
    } else if (duty < 0) {
        pca9685_set_motor_pwm(pwm, ch_b, 0);
        pca9685_set_motor_pwm(pwm, ch_a, -duty);
    } else {
        /* 刹车：双路拉高制动 */
        pca9685_set_motor_pwm(pwm, ch_a, MOTOR_BRAKE_VAL);
        pca9685_set_motor_pwm(pwm, ch_b, MOTOR_BRAKE_VAL);
    }
}

/* ── 初始化与关闭 ────────────────────────────────────── */

/*
 * motor_init - 初始化电机控制器
 * 打开PCA9685，设置PWM频率为50Hz（电机驱动板要求）
 * 对应Python: Ordinary_Car.__init__()
 */
int motor_init(Motor *motor)
{
    if (pca9685_init(&motor->pwm, PCA9685_DEFAULT_ADDR, 1) < 0) {
        fprintf(stderr, "[Motor] PCA9685初始化失败\n");
        return -1;
    }
    pca9685_set_pwm_freq(&motor->pwm, 50);
    printf("[Motor] 初始化完成\n");
    return 0;
}

/*
 * motor_close - 先停车再关闭I2C
 * 对应Python: close()
 */
void motor_close(Motor *motor)
{
    motor_set_model(motor, 0, 0, 0, 0);
    pca9685_close(&motor->pwm);
    printf("[Motor] 已关闭\n");
}

/* ── 占空比截断 ──────────────────────────────────────── */

/*
 * motor_duty_clamp - 将占空比限制在 [-4095, +4095]
 * 对应Python: duty_range()
 */
int motor_duty_clamp(int duty)
{
    if (duty >  MOTOR_DUTY_MAX) return  MOTOR_DUTY_MAX;
    if (duty < -MOTOR_DUTY_MAX) return -MOTOR_DUTY_MAX;
    return duty;
}

/* ── 四轮驱动核心接口 ────────────────────────────────── */

/*
 * motor_set_model - 同时设置四轮速度（核心控制接口）
 *
 * 参数: fl=左前, bl=左后, fr=右前, br=右后
 * 所有值在进入前先做截断处理
 *
 * 通道映射（严格对应Python motor.py 各 *_wheel 函数）:
 *
 *   FL: _wheel_set(0, 1, fl)  → duty>0: ch0=0, ch1=duty  ✓
 *   BL: _wheel_set(3, 2, bl)  → duty>0: ch3=0, ch2=duty  ✓  (注意ch_a=3, ch_b=2)
 *   FR: _wheel_set(6, 7, fr)  → duty>0: ch6=0, ch7=duty  ✓
 *   BR: _wheel_set(4, 5, br)  → duty>0: ch4=0, ch5=duty  ✓
 *
 * 对应Python: set_motor_model(duty1, duty2, duty3, duty4)
 */
void motor_set_model(Motor *motor, int fl, int bl, int fr, int br)
{
    fl = motor_duty_clamp(fl);
    bl = motor_duty_clamp(bl);
    fr = motor_duty_clamp(fr);
    br = motor_duty_clamp(br);

    /* 左前轮 FL: ch_a=0, ch_b=1 */
    _wheel_set(&motor->pwm, 0, 1, fl);

    /* 左后轮 BL: ch_a=3, ch_b=2  ← 注意顺序与FL/FR不同 */
    _wheel_set(&motor->pwm, 3, 2, bl);

    /* 右前轮 FR: ch_a=6, ch_b=7 */
    _wheel_set(&motor->pwm, 6, 7, fr);

    /* 右后轮 BR: ch_a=4, ch_b=5 */
    _wheel_set(&motor->pwm, 4, 5, br);
}
