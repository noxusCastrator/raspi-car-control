/*
 * test_sensors.c — MVP2 全传感器验证测试程序
 *
 * 测试内容:
 *   - ADC:       每0.5秒打印左右光敏电阻和电池电压
 *   - 舵机:      依次转到 0°、90°、180°，再归中
 *   - 超声波:    每0.5秒打印距离（cm）
 *   - 红外:      每0.5秒打印三路值和合并整数
 *   - 蜂鸣器:    响0.2秒 × 3次
 *
 * 用法: sudo ./test_sensors
 * 编译: make test_sensors
 *
 * 注意：运行前确保 main.py 没有在运行（I2C/GPIO冲突）
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "adc.h"
#include "servo.h"
#include "ultrasonic.h"
#include "infrared.h"
#include "buzzer.h"

/* ── 全局句柄（供信号处理器使用）─────────────────────── */
static ADC        g_adc;
static Servo      g_servo;
static Ultrasonic g_sonic;
static Infrared   g_ir;
static Buzzer     g_buzzer;
static int        g_initialized = 0;

/* ── 清理并退出 ──────────────────────────────────────── */
static void cleanup(void)
{
    if (!g_initialized) return;
    printf("\n[Test] 清理资源...\n");
    buzzer_set(&g_buzzer, 0);
    servo_set_angle(&g_servo, 0, 90, 10);   /* 舵机归中 */
    adc_close(&g_adc);
    servo_close(&g_servo);
    ultrasonic_close(&g_sonic);
    infrared_close(&g_ir);
    buzzer_close(&g_buzzer);
    gpioTerminate();   /* pigpio统一在此终止 */
    g_initialized = 0;
}

static void signal_handler(int sig)
{
    (void)sig;
    cleanup();
    exit(0);
}

/* ── 各模块测试函数 ──────────────────────────────────── */

static void test_buzzer(void)
{
    printf("\n[蜂鸣器] 响3次...\n");
    for (int i = 0; i < 3; i++) {
        buzzer_set(&g_buzzer, 1);
        usleep(200000);   /* 0.2s */
        buzzer_set(&g_buzzer, 0);
        usleep(100000);   /* 0.1s */
    }
    printf("[蜂鸣器] 完成\n");
}

static void test_servo(void)
{
    printf("\n[舵机] 测试通道0（超声波云台舵机）...\n");
    printf("  → 30度\n");
    servo_set_angle(&g_servo, 0, 30, 10);
    sleep(1);
    printf("  → 90度（中位）\n");
    servo_set_angle(&g_servo, 0, 90, 10);
    sleep(1);
    printf("  → 150度\n");
    servo_set_angle(&g_servo, 0, 150, 10);
    sleep(1);
    printf("  → 归中 90度\n");
    servo_set_angle(&g_servo, 0, 90, 10);
    sleep(1);
    printf("[舵机] 完成\n");
}

static void test_sensors_loop(int iterations)
{
    printf("\n[传感器循环] 读取 %d 次（每次间隔0.5秒）...\n", iterations);
    printf("%-6s | %-10s %-10s %-10s | %-10s | %-20s\n",
           "次数", "光左(V)", "光右(V)", "电池(V)", "距离(cm)", "红外(bin/int)");
    printf("------+----------------------------------------------+\n");

    for (int i = 0; i < iterations; i++) {
        /* ADC */
        float light_l = adc_read(&g_adc, 0);
        float light_r = adc_read(&g_adc, 1);
        float power   = adc_read(&g_adc, 2);
        /* 电池电压 = ADC值 × 倍数系数（与Python一致）*/
        float battery = power * (g_adc.pcb_version == 1 ? 3.0f : 2.0f);

        /* 超声波 */
        float dist = ultrasonic_get_distance(&g_sonic);

        /* 红外 */
        int ir_all = infrared_read_all(&g_ir);
        int ir1 = (ir_all >> 2) & 1;
        int ir2 = (ir_all >> 1) & 1;
        int ir3 =  ir_all       & 1;

        printf("%-6d | %-10.2f %-10.2f %-10.2f | %-10.1f | %d%d%d (=%d)\n",
               i + 1,
               light_l, light_r, battery,
               dist,
               ir1, ir2, ir3, ir_all);

        usleep(500000);   /* 0.5s */
    }
    printf("[传感器循环] 完成\n");
}

/* ── 主程序 ──────────────────────────────────────────── */
int main(void)
{
    printf("===== MVP2 传感器测试 =====\n");
    printf("按 Ctrl+C 可随时安全退出\n\n");

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    /* ── 初始化所有模块 ── */
    int ok = 1;

    if (adc_init(&g_adc) < 0)                                          { ok = 0; }
    if (ok && servo_init(&g_servo) < 0)                                { ok = 0; }
    if (ok && ultrasonic_init(&g_sonic,
                               ULTRASONIC_TRIGGER_PIN,
                               ULTRASONIC_ECHO_PIN) < 0)               { ok = 0; }
    if (ok && infrared_init(&g_ir) < 0)                                { ok = 0; }
    if (ok && buzzer_init(&g_buzzer) < 0)                              { ok = 0; }

    if (!ok) {
        fprintf(stderr, "\n[Test] 初始化失败，请检查硬件连接和权限（需要sudo）\n");
        fprintf(stderr, "  I2C设备检查: i2cdetect -y 1\n");
        fprintf(stderr, "  应看到: 0x40 (PCA9685)，0x48 (ADS7830)\n");
        cleanup();
        return 1;
    }
    g_initialized = 1;

    printf("\n所有模块初始化成功，开始测试...\n");

    /* ── 按顺序执行各测试 ── */
    test_buzzer();
    test_servo();
    test_sensors_loop(10);

    /* ── 完成 ── */
    cleanup();
    printf("\n===== 测试完成 =====\n");
    printf("若各传感器输出值合理，MVP2通过。\n");
    return 0;
}
