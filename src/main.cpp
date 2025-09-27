#include <Arduino.h>
#include "motors.h"
#include "display.h"
#include "buttons.h"    // 新增：按键模块头文件
#include "ultrasonic.h" // 新增：超声波模块

// Function prototype for clearDisplay
void clearDisplay();

// 业务状态：默认电机关闭、OLED关闭、方向前进
static bool motorEnabled = false;
static bool motorForward = true;
static bool displayEnabled = false;
static const int BASE_SPEED = 200; // 可根据需要调整

// 应用电机状态
static void applyMotorState()
{
  if (!motorEnabled)
  {
    motors(0, 0);
    return;
  }
  int s = motorForward ? BASE_SPEED : -BASE_SPEED;
  motors(s, s);
}

// 新增：按键事件回调（短按）
static void onShortPress(int buttonIndex)
{
  switch (buttonIndex)
  {
  case 0: // K1：电机启动/停止
    motorEnabled = !motorEnabled;
    applyMotorState();
    Serial.println(motorEnabled ? "电机：启动" : "电机：停止");
    break;
  case 1: // K2：OLED 开/关（仅影响是否刷新显示）
    displayEnabled = !displayEnabled;
    if (!displayEnabled)
    {
      clearDisplay(); // 清屏并黑屏
    }
    Serial.println(displayEnabled ? "OLED：显示开启" : "OLED：显示关闭");
    break;
  case 2: // K3：前进/后退
    motorForward = !motorForward;
    applyMotorState();
    Serial.println(motorForward ? "方向：前进" : "方向：后退");
    break;
  case 3: // K4：系统重启
    Serial.println("系统重启中...");
    delay(100);
    ESP.restart();
    break;
  default:
    break;
  }
}

// 新增：按键事件回调（长按）
static void onLongPress(int buttonIndex)
{
  Serial.print("按键");
  Serial.print(buttonIndex + 1);
  Serial.println(" 长按");
}

void setup()
{
  Serial.begin(115200);

  initMotors();

  if (!initDisplay())
  {
    Serial.println(F("SSD1306初始化失败"));
    while (true)
      delay(100);
  }
  else
  {
    Serial.println("OLED 初始化成功");
  }

  Serial.println("TB6612FNG 电机驱动初始化完成");

  // 新增：初始化超声波
  initUltrasonic();

  // 初始化按键（模块化）
  buttonsInit();
  buttonsSetShortPressHandler(onShortPress);
  buttonsSetLongPressHandler(onLongPress);

  // 默认：电机关闭、OLED关闭、方向前进
  motorEnabled = false;
  motorForward = true;
  displayEnabled = false;
  applyMotorState();
}

void loop()
{
  // 按键检测与事件处理（非阻塞，模块化）
  buttonsPoll();

  // OLED 刷新（仅在开启显示时）
  static uint32_t lastUpdate = 0;
  if (displayEnabled && millis() - lastUpdate >= 200)
  {
    // 新增：读取超声波距离，并同时显示距离+两路速度
    float cm = ultrasonicReadCm();
    if (cm < 8.0f)
    {
      motors(0, 0); // 距离过近，紧急停止
      motorEnabled = false;
      Serial.println("距离过近，电机停止");
    }
    else
    {
      motorEnabled = true; // 恢复电机状态
      applyMotorState();
    }
    updateDisplay(cm, getSpeedA(), getSpeedB());
    lastUpdate = millis();
  }

  // 轻微让步，降低CPU占用
  delay(5);
}