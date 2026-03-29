/*
 * test_motor.c — MVP1 验证测试程序
 *
 * 功能：依次执行前进/后退/左转/右转/停止，各持续1秒
 * 用法：sudo ./test_motor
 * 预期：四轮按预期方向转动，控制台打印每步动作，无I2C错误
 *
 * 编译：见 Makefile，执行 make test_motor
 * 注意：需要 sudo 权限（访问 /dev/i2c-1）
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "motor.h"

/* ── 全局电机句柄（供信号处理器使用）─────────────────── */
static Motor g_motor;
static int   g_initialized = 0;

/* ── Ctrl+C 安全退出 ─────────────────────────────────── */
static void signal_handler(int sig)
{
    (void)sig;
    printf("\n[Test] 收到中断信号，停止电机并退出...\n");
    if (g_initialized) {
        motor_set_model(&g_motor, 0, 0, 0, 0);
        motor_close(&g_motor);
    }
    exit(0);
}

/* ── 主程序 ──────────────────────────────────────────── */
int main(void)
{
    printf("===== MVP1 电机测试 =====\n");
    printf("测试序列: 前进→后退→左转→右转→停止，各1秒\n");
    printf("按 Ctrl+C 可随时安全退出\n\n");

    /* 注册信号处理器 */
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    /* 初始化电机 */
    if (motor_init(&g_motor) < 0) {
        fprintf(stderr, "[Test] 电机初始化失败，请检查:\n");
        fprintf(stderr, "  1. 是否以 sudo 运行\n");
        fprintf(stderr, "  2. I2C是否已启用 (raspi-config → Interface → I2C)\n");
        fprintf(stderr, "  3. PCA9685是否正确连接 (i2cdetect -y 1 应显示 0x40)\n");
        return 1;
    }
    g_initialized = 1;

    /* 确认从停止状态开始 */
    printf("[Test] 初始状态: 停止\n");
    motor_set_model(&g_motor, 0, 0, 0, 0);
    sleep(1);

    /* ── 测试序列 ── */

    printf("[Test] 前进 (FL=2000, BL=2000, FR=2000, BR=2000)...\n");
    motor_set_model(&g_motor, 2000, 2000, 2000, 2000);
    sleep(1);

    printf("[Test] 后退 (FL=-2000, BL=-2000, FR=-2000, BR=-2000)...\n");
    motor_set_model(&g_motor, -2000, -2000, -2000, -2000);
    sleep(1);

    printf("[Test] 左转 (FL=-2000, BL=-2000, FR=2000, BR=2000)...\n");
    motor_set_model(&g_motor, -2000, -2000, 2000, 2000);
    sleep(1);

    printf("[Test] 右转 (FL=2000, BL=2000, FR=-2000, BR=-2000)...\n");
    motor_set_model(&g_motor, 2000, 2000, -2000, -2000);
    sleep(1);

    printf("[Test] 停止\n");
    motor_set_model(&g_motor, 0, 0, 0, 0);
    sleep(1);

    /* ── 边界值测试 ── */
    printf("\n[Test] 边界值测试: 占空比截断 (duty=9999, 应被截断至4095)...\n");
    motor_set_model(&g_motor, 9999, 9999, 9999, 9999);
    sleep(1);

    printf("[Test] 停止\n");
    motor_set_model(&g_motor, 0, 0, 0, 0);

    /* 清理 */
    motor_close(&g_motor);
    g_initialized = 0;

    printf("\n===== 测试完成 =====\n");
    printf("若轮子按预期方向转动，MVP1通过。\n");
    return 0;
}
