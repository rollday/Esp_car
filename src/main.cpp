#include <Arduino.h>
#include <math.h>
#include "motors.h"
#include "display.h"
#include "buttons.h" // 新增：按键模块头文件
#include "ultrasonic.h"
#include "mpu.h"

// Function prototype for clearDisplay
void clearDisplay();

// 业务状态：默认电机关闭、OLED关闭、方向前进
static bool motorEnabled = false;
static bool motorForward = true;
static bool displayEnabled = true;
static const int BASE_SPEED = 200; // 可根据需要调整

// 新增：障碍物检测标志
static bool obstacleDetected = false;
static constexpr uint32_t DISPLAY_INTERVAL_MS = 200;
static constexpr int MPU6050_SDA = 47;
static constexpr int MPU6050_SCL = 48;
static unsigned long lastMicros = 0;
static float deltaTime = 0.0f;
static constexpr uint32_t REVERSE_DURATION_MS = 3000;
static constexpr uint32_t PAUSE_DURATION_MS = 2000;
static constexpr float TARGET_YAW_CHANGE = 45.0f;
enum class AvoidState
{
  Idle,
  Reversing,
  PauseAfterReverse,
  Rotating,
  PauseAfterRotate
};
static AvoidState avoidState = AvoidState::Idle;
static unsigned long avoidStateStartMs = 0;
static float avoidInitialYaw = 0.0f;
static float lastDistanceCm = -1.0f;

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

  // 初始化MPU6050
  if (!mpuInit(MPU6050_SDA, MPU6050_SCL))
  {
    Serial.println("找不到MPU6050芯片！");
    while (true)
      delay(10);
  }
  Serial.println("MPU6050初始化成功！");

  // 默认：电机关闭、OLED关闭、方向前进
  motorEnabled = false;
  motorForward = true;
  displayEnabled = false;
  applyMotorState();

  lastMicros = micros();
}

static void updateObstacleAvoidance(float distanceCm, bool hasFreshSample)
{
  if (!motorEnabled && avoidState != AvoidState::Idle)
  {
    motors(0, 0);
    avoidState = AvoidState::Idle;
    obstacleDetected = false;
  }

  if (hasFreshSample)
  {
    if (motorEnabled && distanceCm > 0.0f && distanceCm < 8.0f && avoidState == AvoidState::Idle)
    {
      obstacleDetected = true;
      avoidState = AvoidState::Reversing;
      avoidStateStartMs = millis();
      motors(0, 0);
      Serial.println("距离过近，电机停止");
      motors(-BASE_SPEED, -BASE_SPEED);
      Serial.println("开始后退避障");
    }
    else if (distanceCm >= 8.0f && obstacleDetected && avoidState == AvoidState::Idle)
    {
      obstacleDetected = false;
      Serial.println("障碍物清除");
      if (motorEnabled)
      {
        applyMotorState();
      }
    }
  }

  switch (avoidState)
  {
  case AvoidState::Idle:
    break;
  case AvoidState::Reversing:
    motors(-BASE_SPEED, -BASE_SPEED);
    if (millis() - avoidStateStartMs >= REVERSE_DURATION_MS)
    {
      motors(0, 0);
      avoidState = AvoidState::PauseAfterReverse;
      avoidStateStartMs = millis();
    }
    break;
  case AvoidState::PauseAfterReverse:
    if (millis() - avoidStateStartMs >= PAUSE_DURATION_MS)
    {
      avoidInitialYaw = mpuGetState().yaw;
      avoidState = AvoidState::Rotating;
    }
    break;
  case AvoidState::Rotating:
    motors(BASE_SPEED, -BASE_SPEED);
    if (fabsf(mpuGetState().yaw - avoidInitialYaw) >= TARGET_YAW_CHANGE)
    {
      motors(0, 0);
      avoidState = AvoidState::PauseAfterRotate;
      avoidStateStartMs = millis();
      Serial.println("原地转向完成");
    }
    break;
  case AvoidState::PauseAfterRotate:
    if (millis() - avoidStateStartMs >= PAUSE_DURATION_MS)
    {
      avoidState = AvoidState::Idle;
      obstacleDetected = false;
      Serial.println("尝试避障完成，继续前进");
      if (motorEnabled)
      {
        applyMotorState();
      }
    }
    break;
  }
}

void loop()
{
  const unsigned long nowMicros = micros();
  deltaTime = (nowMicros - lastMicros) / 1000000.0f;
  lastMicros = nowMicros;
  mpuUpdate(deltaTime);

  // 按键检测与事件处理（非阻塞，模块化）
  buttonsPoll();

  float distanceForAvoidance = lastDistanceCm;
  bool hasFreshDistance = false;

  static uint32_t lastUpdate = 0;
  if (displayEnabled && millis() - lastUpdate >= DISPLAY_INTERVAL_MS)
  {
    float cm = ultrasonicReadCm();
    if (cm >= 0.0f)
    {
      lastDistanceCm = cm;
      distanceForAvoidance = cm;
      hasFreshDistance = true;
    }
    else
    {
      cm = lastDistanceCm;
    }

    if (avoidState == AvoidState::Idle)
    {
      if (cm < 15.0f && cm >= 8.0f && motorEnabled)
      {
        Serial.println("前方有障碍物，差速转弯通过");
        const float TURN_RATIO = 0.5f;
        motors(BASE_SPEED, BASE_SPEED * TURN_RATIO);
      }
      else if (cm >= 15.0f)
      {
        Serial.println("无障碍物，继续同速度前进");
        applyMotorState();
      }
    }

    const MpuState &mpuState = mpuGetState();
    float planarVelocity = hypotf(mpuState.velocityX, mpuState.velocityY);
    updateDisplay(cm, motorEnabled, motorForward, planarVelocity);
    lastUpdate = millis();
  }

  updateObstacleAvoidance(distanceForAvoidance, hasFreshDistance);

  // 轻微让步，降低CPU占用
  delay(5);
}