/*
 * buzzer.c — 蜂鸣器驱动实现
 *
 * 对应Python: buzzer.py (class Buzzer)
 * GPIO库: pigpio
 */

#include "buzzer.h"

#include <stdio.h>
#include <pigpio.h>

/* ── 初始化 ──────────────────────────────────────────── */

int buzzer_init(Buzzer *bz)
{
    bz->pin = BUZZER_PIN;

    if (gpioInitialise() < 0) {
        fprintf(stderr, "[Buzzer] pigpio初始化失败\n");
        return -1;
    }

    gpioSetMode(bz->pin, PI_OUTPUT);
    gpioWrite(bz->pin, 0);  /* 初始关闭 */

    printf("[Buzzer] 初始化完成，引脚=GPIO%d\n", bz->pin);
    return 0;
}

/* ── 控制 ────────────────────────────────────────────── */

/*
 * buzzer_set - 设置蜂鸣器开/关
 * 对应Python: set_state(state)
 */
void buzzer_set(Buzzer *bz, int state)
{
    gpioWrite(bz->pin, state ? 1 : 0);
}

/* ── 关闭 ────────────────────────────────────────────── */

void buzzer_close(Buzzer *bz)
{
    gpioWrite(bz->pin, 0);         /* 确保关闭 */
    gpioSetMode(bz->pin, PI_INPUT);
    printf("[Buzzer] 已关闭\n");
}
