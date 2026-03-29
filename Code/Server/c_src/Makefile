# Makefile — Freenove 4WD Smart Car C服务端
#
# 用法:
#   make              — 编译当前阶段所有目标（默认=test_motor）
#   make test_motor   — 编译MVP1电机测试程序
#   make clean        — 清理所有编译产物
#
# 依赖安装（树莓派上执行一次）:
#   sudo apt update
#   sudo apt install gcc make libi2c-dev pigpio libpigpio-dev
#
# 运行需要sudo（访问 /dev/i2c-1 和 GPIO）:
#   sudo ./test_motor

# ── 编译器与标志 ──────────────────────────────────────
CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -g
LIBS    = -lm

# ── MVP阶段源文件（逐步追加）─────────────────────────
# MVP1: PCA9685驱动 + 电机控制
MVP1_SRCS = pca9685.c motor.c
MVP1_OBJS = $(MVP1_SRCS:.c=.o)

# MVP2: + 舵机 + ADC + 传感器（TODO: 后续添加）
# MVP2_SRCS = $(MVP1_SRCS) servo.c adc.c ultrasonic.c infrared.c buzzer.c
# MVP2_OBJS = $(MVP2_SRCS:.c=.o)

# MVP3: + TCP服务器 + 协议解析（TODO: 后续添加）
# MVP3_SRCS = $(MVP2_SRCS) tcp_server.c message.c
# MVP3_LIBS = $(LIBS) -lpthread

# MVP4: + 小车逻辑（TODO: 后续添加）
# MVP4_SRCS = $(MVP3_SRCS) car.c
# MVP4_LIBS = $(MVP3_LIBS) -lpigpio -lrt

# ── 默认目标 ─────────────────────────────────────────
.PHONY: all clean test_motor

all: test_motor

# ── MVP1: 电机测试 ───────────────────────────────────
test_motor: test_motor.c $(MVP1_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo ""
	@echo "编译成功: ./test_motor"
	@echo "运行方式: sudo ./test_motor"

# ── 通用编译规则 ─────────────────────────────────────
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ── 头文件依赖 ────────────────────────────────────────
pca9685.o: pca9685.c pca9685.h
motor.o:   motor.c   motor.h pca9685.h

# ── 清理 ─────────────────────────────────────────────
clean:
	rm -f *.o test_motor
	@echo "清理完成"
