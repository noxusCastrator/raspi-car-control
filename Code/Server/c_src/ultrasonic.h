/*
 * ultrasonic.h — 超声波测距传感器驱动（HC-SR04）
 *
 * 对应Python: ultrasonic.py (class Ultrasonic)
 * GPIO库: pigpio（需要 pigpiod 后台进程或 gpioInitialise()）
 *
 * 硬件连接:
 *   TRIG → GPIO 27
 *   ECHO → GPIO 22
 *
 * 测距原理:
 *   1. 向TRIG发送 ≥10us 高电平脉冲
 *   2. 等待ECHO变高（超声波发出）
 *   3. 记录ECHO高电平持续时间（us）
 *   4. 距离(cm) = 时间(us) * 声速(cm/us) / 2
 *              = 时间(us) * 0.017
 *
 * 超时保护: 等待超过 30ms 则视为无回波，返回最大距离值
 */

#ifndef ULTRASONIC_H
#define ULTRASONIC_H

/* ── 引脚与参数 ─────────────────────────────────────── */
#define ULTRASONIC_TRIGGER_PIN  27
#define ULTRASONIC_ECHO_PIN     22
#define ULTRASONIC_TIMEOUT_US   30000   /* 超时 30ms，对应最大距离约 510cm */
#define ULTRASONIC_MAX_DIST_CM  300.0f  /* 超时时返回此值 */
#define ULTRASONIC_SOUND_SPEED  0.017f  /* cm/us（20°C空气中约340m/s） */

typedef struct {
    int trigger_pin;
    int echo_pin;
} Ultrasonic;

/**
 * ultrasonic_init - 初始化超声波传感器，配置GPIO方向
 * @us:          传感器句柄指针
 * @trigger_pin: TRIG引脚（默认 ULTRASONIC_TRIGGER_PIN）
 * @echo_pin:    ECHO引脚（默认 ULTRASONIC_ECHO_PIN）
 * 返回: 0=成功, -1=失败（pigpio初始化失败）
 * 注意: 内部调用 gpioInitialise()，可重复调用
 * 对应Python: Ultrasonic.__init__()
 */
int ultrasonic_init(Ultrasonic *us, int trigger_pin, int echo_pin);

/**
 * ultrasonic_get_distance - 获取当前距离（厘米）
 * @us: 传感器句柄
 * 返回: 距离（cm），无回波时返回 ULTRASONIC_MAX_DIST_CM
 * 对应Python: get_distance()
 */
float ultrasonic_get_distance(Ultrasonic *us);

/**
 * ultrasonic_close - 释放GPIO资源
 * @us: 传感器句柄
 * 对应Python: close()
 */
void ultrasonic_close(Ultrasonic *us);

#endif /* ULTRASONIC_H */
