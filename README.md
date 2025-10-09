# ESP Car 控制项目（ESP32-S3 N16R8）

面向 ESP32-S3-DevKitM-1（16 MB Flash / 8 MB PSRAM）的 PlatformIO + Arduino 智能小车示例，整合双路 TB6612FNG 电机驱动、OLED 显示屏、HC-SR04 超声波测距、K1~K4 实体按键与 MPU6050 姿态传感器。主循环遵循非阻塞设计，以全局状态协调电机、显示、避障与计时器逻辑。

## 硬件映射

| 模块 | 引脚 | 说明 |
|------|------|------|
| TB6612FNG A路 | AIN1=GPIO10, AIN2=GPIO9, PWMA=GPIO46 | PWM 1 kHz / 8 bit |
| TB6612FNG B路 | BIN1=GPIO12, BIN2=GPIO13, PWMB=GPIO14 | 同步保持通道 0/1 |
| STBY | GPIO11 | 高电平唤醒驱动 |
| OLED SSD1306 | I²C 0x3C, SDA=GPIO5, SCL=GPIO4 | 使用 `Wire.begin(5, 4)` |
| 按键 K1~K4 | GPIO15~GPIO18（上拉输入） | 低电平按下 |
| 超声波 | TRIG=GPIO20, ECHO=GPIO21 | 30 ms 超时 |
| MPU6050 | SDA=GPIO47, SCL=GPIO48 | 采用 Adafruit 库 |

## 目录结构

```
Esp_test/
├─ .github/                 # Copilot 策略
├─ include/                 # 公共头文件
├─ src/
│  ├─ main.cpp              # 应用入口与状态机
│  ├─ motors.*              # 电机控制封装
│  ├─ display.*             # OLED 显示管理
│  ├─ buttons.*             # 按键去抖与事件
│  ├─ ultrasonic.*          # 超声波读数
│  └─ mpu.*                 # MPU6050 姿态估计
├─ lib/                     # 额外自定义库（可选）
└─ platformio.ini           # PlatformIO 配置
```

## 核心功能

- **电机控制**：`motors()` 接管 TB6612FNG，`BASE_SPEED=200`，方向由 `motorForward` 决定。
- **避障状态机**：< 8 cm 触发后退 → 暂停 → 原地转向 45° → 再暂停；8~15 cm 实时差速转弯，>15 cm 巡航。
- **显示系统**：每 200 ms 刷新距离、速度、启停状态；关闭显示调用 `clearDisplay()` 黑屏。
- **按键语义**：
  - K1：启停电机
  - K2：OLED 开关
  - K3：正反转切换
  - K4：`ESP.restart()`
  - 长按仅输出串口日志
- **姿态估计**：MPU6050 卡尔曼融合滚转/俯仰/偏航，并估算平面速度向量。

## 快速上手

1. **环境准备**
   - 安装 VS Code 与 PlatformIO IDE 扩展。
   - 确认已安装 `Adafruit SSD1306/GFX`、`Adafruit MPU6050` 等依赖。
2. **获取代码**
   ```bash
   git clone <repo-url>
   cd Esp_test
   ```
3. **构建与烧录**
   - 构建：`pio run`
   - 上传：`pio run -t upload`（串口 115200，上载速率 2000000）
   - 监视：`pio device monitor -b 115200`
4. **硬件连线**
   - 确保电机、电源与传感器按上表接线。
   - I²C 总线若无板载上拉，请外接 4.7 kΩ。

## 运行流程

1. `setup()` 初始化电机、OLED、按键、超声波与 MPU6050。
2. `loop()` 内部步骤：
   - 读取 MPU 数据更新姿态与速度。
   - 轮询按键触发短按/长按回调。
   - 定期读取超声波距离，执行避障状态机。
   - 按需刷新 OLED 显示。
   - `delay(5)` 让步以维持非阻塞循环。

## 调试与常见问题

- **OLED 无显示**：检查 I²C 引脚、供电与库版本；串口将提示初始化失败。
- **按键误触**：确认 20 ms 去抖与 500 ms 上电稳定期未被跳过。
- **避障误判**：确保超声波电压兼容 3.3 V；可在串口查看距离日志。
- **上传失败**：更换数据线或降低上传速率；如占用串口请关闭监视器。

## 许可

使用与分发请遵循仓库内 LICENSE 规定。
