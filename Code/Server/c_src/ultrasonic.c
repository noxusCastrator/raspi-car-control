/*
 * ultrasonic.c — 超声波测距传感器驱动实现（HC-SR04）
 *
 * 对应Python: ultrasonic.py (class Ultrasonic)
 * GPIO库: pigpio
 *
 * 测距流程:
 *   1. TRIG 拉低 2us（确保干净起始状态）
 *   2. TRIG 拉高 10us（触发超声波发射）
 *   3. TRIG 拉低
 *   4. 等待 ECHO 变高（超声波在空气中传播）
 *   5. 记录 ECHO 变高时刻 t_start（gpioTick，微秒）
 *   6. 等待 ECHO 变低（超声波返回）
 *   7. 记录 ECHO 变低时刻 t_end
 *   8. distance(cm) = (t_end - t_start) * 0.017
 *
 * 超时处理: 任意等待阶段超过 30ms 则返回最大距离值
 */

#include "ultrasonic.h"

#include <stdio.h>
#include <pigpio.h>

/* ── 初始化 ──────────────────────────────────────────── */

int ultrasonic_init(Ultrasonic *us, int trigger_pin, int echo_pin)
{
    us->trigger_pin = trigger_pin;
    us->echo_pin    = echo_pin;

    /* pigpio初始化（可重复调用，已初始化则直接返回版本号）*/
    if (gpioInitialise() < 0) {
        fprintf(stderr, "[Ultrasonic] pigpio初始化失败\n");
        fprintf(stderr, "  请确认: sudo pigpiod 已运行，或以 sudo 执行本程序\n");
        return -1;
    }

    gpioSetMode(us->trigger_pin, PI_OUTPUT);
    gpioSetMode(us->echo_pin,    PI_INPUT);

    /* TRIG 初始拉低 */
    gpioWrite(us->trigger_pin, 0);

    printf("[Ultrasonic] 初始化完成，TRIG=GPIO%d，ECHO=GPIO%d\n",
           us->trigger_pin, us->echo_pin);
    return 0;
}

/* ── 测距 ────────────────────────────────────────────── */

float ultrasonic_get_distance(Ultrasonic *us)
{
    uint32_t t_start, t_end, t_timeout;

    /* ── Step 1: 发送触发脉冲 ── */
    gpioWrite(us->trigger_pin, 0);
    gpioDelay(2);                       /* 2us 低电平，确保干净 */
    gpioWrite(us->trigger_pin, 1);
    gpioDelay(10);                      /* 10us 高电平触发 */
    gpioWrite(us->trigger_pin, 0);

    /* ── Step 2: 等待 ECHO 变高（超声波发出）── */
    t_timeout = gpioTick();
    while (gpioRead(us->echo_pin) == 0) {
        if (gpioTick() - t_timeout > (uint32_t)ULTRASONIC_TIMEOUT_US) {
            /* 等待超时，传感器无响应 */
            return ULTRASONIC_MAX_DIST_CM;
        }
    }
    t_start = gpioTick();

    /* ── Step 3: 等待 ECHO 变低（超声波返回）── */
    while (gpioRead(us->echo_pin) == 1) {
        if (gpioTick() - t_start > (uint32_t)ULTRASONIC_TIMEOUT_US) {
            /* 回波超时，目标超出量程 */
            return ULTRASONIC_MAX_DIST_CM;
        }
    }
    t_end = gpioTick();

    /* ── Step 4: 计算距离 ── */
    uint32_t duration_us = t_end - t_start;
    float distance = (float)duration_us * ULTRASONIC_SOUND_SPEED;

    return distance;
}

/* ── 关闭 ────────────────────────────────────────────── */

void ultrasonic_close(Ultrasonic *us)
{
    /* 将引脚复位为输入（释放驱动状态）*/
    gpioSetMode(us->trigger_pin, PI_INPUT);
    printf("[Ultrasonic] 已关闭\n");
    /* 注意：gpioTerminate() 由 main 统一调用，不在此处调用 */
}
