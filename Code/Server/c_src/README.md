# c_src — C服务端代码

Freenove 4WD Smart Car 的C移植部分，运行在树莓派上。

---

## 第一次使用：环境准备

以下命令在树莓派上执行一次即可。

**1. 启用 I2C 接口**
```bash
sudo raspi-config
# 依次选择: Interface Options → I2C → Enable
# 重启生效
sudo reboot
```

**2. 确认 PCA9685 已连接**
```bash
i2cdetect -y 1
# 正常输出中应能看到 0x40（电机/舵机控制板）
```

**3. 确认 ADS7830 已连接（ADC芯片）**
```bash
i2cdetect -y 1
# 应能同时看到 0x40（PCA9685）和 0x48（ADS7830）
```

**4. 安装编译依赖**
```bash
sudo apt update
sudo apt install gcc make pigpio libpigpio-dev
```

---

## 编译

```bash
cd c_src
make
```

---

## MVP 测试说明

每个 MVP 阶段有对应的测试程序，**运行前请确保 main.py 没有在运行**（两者同时操作 I2C 会冲突）。

```bash
# 检查并停止 main.py
ps aux | grep main.py
sudo pkill -f main.py
```

### MVP 1 — 电机测试

验证 PCA9685 驱动和四轮电机控制是否正常。

```bash
make test_motor
sudo ./test_motor
```

**预期行为**：小车依次前进 → 后退 → 左转 → 右转 → 停止，各持续 1 秒。

### MVP 2 — 全传感器测试

验证 ADC、舵机、超声波、红外、蜂鸣器全部正常。

```bash
make test_sensors
sudo ./test_sensors
```

**预期行为**：
1. 蜂鸣器响 3 次
2. 舵机转到 30° → 90° → 150° → 归中
3. 循环 10 次打印：左右光敏电压 / 电池电压 / 超声波距离(cm) / 红外三路值

---

## 文件说明

| 文件 | 说明 |
|------|------|
| `pca9685.h/c` | PCA9685 PWM芯片 I2C驱动（对应 `pca9685.py`）|
| `motor.h/c` | 四轮电机控制（对应 `motor.py`）|
| `parameter.h/c` | 读取 params.json 硬件配置（对应 `parameter.py`）|
| `adc.h/c` | ADS7830 ADC I2C驱动（对应 `adc.py`）|
| `servo.h/c` | 舵机控制（对应 `servo.py`）|
| `ultrasonic.h/c` | 超声波测距，使用 pigpio（对应 `ultrasonic.py`）|
| `infrared.h/c` | 三路红外循迹，使用 pigpio（对应 `infrared.py`）|
| `buzzer.h/c` | 蜂鸣器，使用 pigpio（对应 `buzzer.py`）|
| `test_motor.c` | MVP1 验证程序 |
| `test_sensors.c` | MVP2 验证程序 |
| `Makefile` | 构建文件 |

---

## 常见问题

**`Failed to open /dev/i2c-1`**
→ I2C 未启用，参考上方"启用 I2C 接口"步骤。

**`Failed to set I2C slave address 0x40`**
→ PCA9685 未连接，用 `i2cdetect -y 1` 确认。

**`Failed to set I2C slave address 0x48`**
→ ADS7830 未连接，用 `i2cdetect -y 1` 确认。

**`pigpio初始化失败`**
→ 安装 pigpio：`sudo apt install pigpio libpigpio-dev`

**`Permission denied`**
→ 需要 sudo：`sudo ./test_sensors`

**轮子方向不对**
→ 检查电机线是否接反，或告知开发者核对通道映射。

**传感器值全为 0 或异常**
→ 检查对应 GPIO 引脚连接：TRIG=GPIO27, ECHO=GPIO22, IR=GPIO14/15/23, Buzzer=GPIO17
