/*
 * infrared.c — 三路红外循迹传感器驱动实现
 *
 * 对应Python: infrared.py (class Infrared)
 * GPIO库: pigpio
 *
 * 读值约定:
 *   gpioRead() 返回 0 或 1
 *   Python LineSensor: value=1 表示检测到黑线（sensor active）
 *   LineSensor 的 value 属性: 检测到线 → True(1)，未检测到 → False(0)
 *   此处直接读取 GPIO 电平，与 LineSensor 行为一致
 */

#include "infrared.h"

#include <stdio.h>
#include <pigpio.h>

/* ── 初始化 ──────────────────────────────────────────── */

int infrared_init(Infrared *ir)
{
    ir->pin1 = INFRARED_PIN_1;
    ir->pin2 = INFRARED_PIN_2;
    ir->pin3 = INFRARED_PIN_3;

    if (gpioInitialise() < 0) {
        fprintf(stderr, "[Infrared] pigpio初始化失败\n");
        return -1;
    }

    gpioSetMode(ir->pin1, PI_INPUT);
    gpioSetMode(ir->pin2, PI_INPUT);
    gpioSetMode(ir->pin3, PI_INPUT);

    printf("[Infrared] 初始化完成，引脚: ch1=GPIO%d, ch2=GPIO%d, ch3=GPIO%d\n",
           ir->pin1, ir->pin2, ir->pin3);
    return 0;
}

/* ── 单路读取 ────────────────────────────────────────── */

/*
 * infrared_read_one - 读取单路传感器
 * 对应Python: read_one_infrared(channel)
 *
 * channel: 1=左(pin14), 2=中(pin15), 3=右(pin23)
 */
int infrared_read_one(Infrared *ir, int channel)
{
    int pin;
    switch (channel) {
        case 1: pin = ir->pin1; break;
        case 2: pin = ir->pin2; break;
        case 3: pin = ir->pin3; break;
        default:
            fprintf(stderr, "[Infrared] 无效通道: %d（有效: 1, 2, 3）\n", channel);
            return 0;
    }
    return gpioRead(pin);
}

/* ── 三路合并读取 ────────────────────────────────────── */

/*
 * infrared_read_all - 三路合并为3位整数
 *
 * 位排列（与Python一致）:
 *   bit2 = ch1（左）
 *   bit1 = ch2（中）
 *   bit0 = ch3（右）
 *
 * 对应Python:
 *   return (read_one(1) << 2) | (read_one(2) << 1) | read_one(3)
 */
int infrared_read_all(Infrared *ir)
{
    int v1 = gpioRead(ir->pin1);
    int v2 = gpioRead(ir->pin2);
    int v3 = gpioRead(ir->pin3);
    return (v1 << 2) | (v2 << 1) | v3;
}

/* ── 关闭 ────────────────────────────────────────────── */

void infrared_close(Infrared *ir)
{
    /* 复位为输入（高阻态）*/
    gpioSetMode(ir->pin1, PI_INPUT);
    gpioSetMode(ir->pin2, PI_INPUT);
    gpioSetMode(ir->pin3, PI_INPUT);
    printf("[Infrared] 已关闭\n");
}
