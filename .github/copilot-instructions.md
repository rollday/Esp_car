# ESP Car Copilot 指南

## 项目速览
- 基于 PlatformIO 的 Arduino 项目（配置见 `platformio.ini`），目标板为 `esp32-s3-devkitm-1`，串口速率 115200，上传速率 2000000。
- 核心入口在 `src/main.cpp`：通过全局状态驱动电机、OLED、超声波与按键事件，循环中保持非阻塞轮询并以 `delay(5)` 让步。

## 模块职责
- `motors.{h,cpp}`：封装 TB6612FNG 双路电机控制，使用 `ledcSetup` 配置 1 kHz/8 位 PWM。`motors()` 会调用 `motorA/B` 并保持在 ±255 范围，`getSpeedA/B` 返回最近一次请求的速度值。
- `display.{h,cpp}`：基于 Adafruit SSD1306/GFX，通过 `Wire.begin(SDA=5, SCL=4)` 使用 I²C。`initDisplay()` 设置一次 `isDisplayInitialized` 标志；`updateDisplay` 有三种重载，主循环调用 `updateDisplay(distance, speedA, speedB)`。
- `buttons.{h,cpp}`：管理 K1–K4 按键，默认上拉输入，含 20 ms 去抖与 500 ms 上电稳定时间。`buttonsSetShortPressHandler`/`buttonsSetLongPressHandler` 注册回调，短按在松开且 500 ms 内触发，长按阈值 1 s。
- `ultrasonic.{h,cpp}`：固定 `ULTRASONIC_TRIG=20`,`ULTRASONIC_ECHO=21`，`ultrasonicReadCm()` 使用 `pulseIn`（30 ms 超时）并返回厘米距离，失败返回 -1.0。

## 行为约定
- 全局状态：`motorEnabled`、`motorForward`、`displayEnabled` 控制电机与 OLED。`BASE_SPEED` 目前为 200，按钮 K3 翻转方向触发 ±200 PWM。
- 安全策略：OLED 刷新周期为 200 ms；若超声波距离 `< 8.0f`，主循环会立即 `motors(0,0)`、关闭 `motorEnabled` 并打印提示。
- 按键语义：K1 切换电机启停、K2 切换显示（关闭时调用 `clearDisplay()` 黑屏）、K3 反转方向、K4 通过 `ESP.restart()` 复位；长按仅串口日志，不改变状态。
- 显示格式：第一行显示距离（单位 cm），第二行以 `V: xxxx xxxx` 展示两路速度，新增模式需保持在 `isDisplayInitialized` 检查之后绘制并调用 `display.display()`。

## 硬件与引脚
- 电机驱动：AIN1=10, AIN2=9, PWMA=46；BIN1=12, BIN2=13, PWMB=14；待机引脚 STBY=11。扩展时保持与 `ledcAttachPin` 的通道（A=0，B=1）一致。
- OLED：I²C 地址 0x3C，SDA=5，SCL=4。
- 按键：K1–K4 分别为 GPIO 15–18（输入上拉，低电平按下）。
- 超声波：Trig=GPIO20，Echo=GPIO21，使用 3.3 V 兼容模块。

## 开发者工作流
- 构建/上传：使用 PlatformIO 任务或命令行 `pio run`、`pio run -t upload`，串口监视器 `pio device monitor -b 115200`。
- 典型调试：串口日志在 115200 波特率，通过 `Serial.println` 输出。需要模拟传感器时，可在模块内提供假数据但务必保护 `isDisplayInitialized` 与 `ultrasonicInited` 标志。
- 新增行为时遵循非阻塞循环：在 `loop()` 中复用现有 `buttonsPoll()` / `lastUpdate` 节奏，不要引入长阻塞延迟。

## 扩展建议
- 若添加新传感器或驾驶模式，优先在各自模块中扩展函数并在 `main.cpp` 中集中协调，保持单一事件循环。
- 修改引脚或速度时同步更新头文件常量并校对 `init*` 函数，避免散落硬编码。
