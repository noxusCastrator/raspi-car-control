/*
 * buzzer.h — 蜂鸣器驱动
 *
 * 对应Python: buzzer.py (class Buzzer)
 * GPIO库: pigpio
 *
 * 硬件连接: GPIO 17
 */

#ifndef BUZZER_H
#define BUZZER_H

#define BUZZER_PIN 17

typedef struct {
    int pin;
} Buzzer;

/**
 * buzzer_init - 初始化蜂鸣器，配置GPIO为输出并默认关闭
 * @bz: 蜂鸣器句柄指针
 * 返回: 0=成功, -1=失败
 * 注意: 内部调用 gpioInitialise()
 * 对应Python: Buzzer.__init__()
 */
int buzzer_init(Buzzer *bz);

/**
 * buzzer_set - 设置蜂鸣器开/关
 * @bz:    蜂鸣器句柄
 * @state: 1=开, 0=关
 * 对应Python: set_state(state)
 */
void buzzer_set(Buzzer *bz, int state);

/**
 * buzzer_close - 关闭蜂鸣器并释放GPIO
 * @bz: 蜂鸣器句柄
 * 对应Python: close()
 */
void buzzer_close(Buzzer *bz);

#endif /* BUZZER_H */
