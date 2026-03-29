/*
 * infrared.h — 三路红外循迹传感器驱动
 *
 * 对应Python: infrared.py (class Infrared)
 * GPIO库: pigpio
 *
 * 硬件连接:
 *   传感器1（左）  → GPIO 14
 *   传感器2（中）  → GPIO 15
 *   传感器3（右）  → GPIO 23
 *
 * 返回值约定（与Python一致）:
 *   检测到黑线 → 1，未检测到 → 0
 *   read_all() 将三路合并为3位整数:
 *     bit2=传感器1, bit1=传感器2, bit0=传感器3
 *
 *   常见值含义（car.py mode_infrared 使用）:
 *     2 (010) → 中间检测到，前进
 *     4 (100) → 左边检测到，右转
 *     1 (001) → 右边检测到，左转
 *     6 (110) → 左+中，大幅右转
 *     3 (011) → 中+右，大幅左转
 *     7 (111) → 全检测到，停止
 */

#ifndef INFRARED_H
#define INFRARED_H

/* ── 引脚定义 ────────────────────────────────────────── */
#define INFRARED_PIN_1  14  /* 左 */
#define INFRARED_PIN_2  15  /* 中 */
#define INFRARED_PIN_3  23  /* 右 */

typedef struct {
    int pin1;   /* 传感器1 GPIO引脚 */
    int pin2;   /* 传感器2 GPIO引脚 */
    int pin3;   /* 传感器3 GPIO引脚 */
} Infrared;

/**
 * infrared_init - 初始化三路红外传感器，配置GPIO为输入
 * @ir: 传感器句柄指针
 * 返回: 0=成功, -1=失败
 * 注意: 内部调用 gpioInitialise()
 * 对应Python: Infrared.__init__()
 */
int infrared_init(Infrared *ir);

/**
 * infrared_read_one - 读取单路传感器值
 * @ir:      传感器句柄
 * @channel: 通道号（1=左, 2=中, 3=右）
 * 返回: 0 或 1
 * 对应Python: read_one_infrared(channel)
 */
int infrared_read_one(Infrared *ir, int channel);

/**
 * infrared_read_all - 读取三路合并值
 * @ir: 传感器句柄
 * 返回: 3位整数（0~7），bit2=ch1, bit1=ch2, bit0=ch3
 * 对应Python: read_all_infrared()
 */
int infrared_read_all(Infrared *ir);

/**
 * infrared_close - 释放GPIO资源
 * @ir: 传感器句柄
 * 对应Python: close()
 */
void infrared_close(Infrared *ir);

#endif /* INFRARED_H */
