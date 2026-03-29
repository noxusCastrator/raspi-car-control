# Makefile — Freenove 4WD Smart Car C服务端
#
# 用法:
#   make                — 编译所有当前测试目标
#   make test_motor     — MVP1：电机测试（仅I2C，无需pigpio）
#   make test_sensors   — MVP2：全传感器测试（需要pigpio）
#   make clean          — 清理编译产物
#
# 依赖安装（树莓派上执行一次）:
#   sudo apt update
#   sudo apt install gcc make pigpio libpigpio-dev
#
# 运行需要 sudo（访问 /dev/i2c-1 和 GPIO）:
#   sudo ./test_motor
#   sudo ./test_sensors

# ── 编译器与标志 ──────────────────────────────────────
CC     = gcc
CFLAGS = -Wall -Wextra -O2 -g

# ── 依赖库 ────────────────────────────────────────────
LIBS_BASE = -lm
LIBS_GPIO = -lpigpio -lrt
LIBS_ALL  = $(LIBS_BASE) $(LIBS_GPIO)

# ── MVP1 源文件（仅I2C，不需要pigpio）────────────────
MVP1_SRCS = pca9685.c motor.c
MVP1_OBJS = $(MVP1_SRCS:.c=.o)

# ── MVP2 新增源文件 ───────────────────────────────────
MVP2_NEW_SRCS = parameter.c adc.c servo.c ultrasonic.c infrared.c buzzer.c
MVP2_NEW_OBJS = $(MVP2_NEW_SRCS:.c=.o)
MVP2_SRCS     = $(MVP1_SRCS) $(MVP2_NEW_SRCS)
MVP2_OBJS     = $(MVP2_SRCS:.c=.o)

# MVP3: + TCP服务器 + 协议解析（TODO）
# MVP3_SRCS = $(MVP2_SRCS) tcp_server.c message.c
# MVP3_LIBS = $(LIBS_ALL) -lpthread

# MVP4: + 小车逻辑（TODO）
# MVP4_SRCS = $(MVP3_SRCS) car.c

# ── 默认目标 ─────────────────────────────────────────
.PHONY: all clean test_motor test_sensors

all: test_motor test_sensors

# ── MVP1: 电机测试 ───────────────────────────────────
test_motor: test_motor.c $(MVP1_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS_BASE)
	@echo "编译成功 → sudo ./test_motor"

# ── MVP2: 全传感器测试 ────────────────────────────────
test_sensors: test_sensors.c $(MVP2_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS_ALL)
	@echo "编译成功 → sudo ./test_sensors"

# ── 通用编译规则 ─────────────────────────────────────
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ── 头文件依赖声明 ────────────────────────────────────
pca9685.o:    pca9685.c    pca9685.h
motor.o:      motor.c      motor.h      pca9685.h
parameter.o:  parameter.c  parameter.h
adc.o:        adc.c        adc.h        parameter.h
servo.o:      servo.c      servo.h      pca9685.h
ultrasonic.o: ultrasonic.c ultrasonic.h
infrared.o:   infrared.c   infrared.h
buzzer.o:     buzzer.c     buzzer.h

# ── 清理 ─────────────────────────────────────────────
clean:
	rm -f *.o test_motor test_sensors
	@echo "清理完成"
