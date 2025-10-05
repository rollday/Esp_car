#include "app_logic.h"
#include <Arduino.h>
#include "motors.h"

namespace {
  AppState *gState = nullptr;
  ClearDisplayFn gClearDisplay = nullptr;
  constexpr int BASE_SPEED = 200;
}

void appLogicInit(AppState &state, ClearDisplayFn clearFn) {
  gState = &state;
  gClearDisplay = clearFn;
  gState->motorEnabled = false;
  gState->motorForward = true;
  gState->displayEnabled = false;
  gState->obstacleDetected = false;
}

const AppState &appLogicGetState() {
  static AppState dummy{};
  return gState ? *gState : dummy;
}

void appLogicApplyMotorState() {
  if (!gState) {
    return;
  }
  if (!gState->motorEnabled) {
    motors(0, 0);
    return;
  }
  const int speed = gState->motorForward ? BASE_SPEED : -BASE_SPEED;
  motors(speed, speed);
}

void appLogicOnShortPress(int buttonIndex) {
  if (!gState) {
    return;
  }
  switch (buttonIndex) {
  case 0:
    gState->motorEnabled = !gState->motorEnabled;
    appLogicApplyMotorState();
    Serial.println(gState->motorEnabled ? "电机：启动" : "电机：停止");
    break;
  case 1:
    gState->displayEnabled = !gState->displayEnabled;
    if (!gState->displayEnabled && gClearDisplay) {
      gClearDisplay();
    }
    Serial.println(gState->displayEnabled ? "OLED：显示开启" : "OLED：显示关闭");
    break;
  case 2:
    gState->motorForward = !gState->motorForward;
    appLogicApplyMotorState();
    Serial.println(gState->motorForward ? "方向：前进" : "方向：后退");
    break;
  case 3:
    Serial.println("系统重启中...");
    delay(100);
    ESP.restart();
    break;
  default:
    break;
  }
}

void appLogicOnLongPress(int buttonIndex) {
  Serial.print("按键");
  Serial.print(buttonIndex + 1);
  Serial.println(" 长按");
}

void appLogicHandleObstacle(float distanceCm) {
  if (!gState) {
    return;
  }
  if (distanceCm < 8.0f) {
    if (!gState->obstacleDetected) {
      motors(0, 0);
      gState->obstacleDetected = true;
      Serial.println("距离过近，电机停止");
    }
  } else {
    if (gState->obstacleDetected) {
      gState->obstacleDetected = false;
      Serial.println("障碍物清除");
    }
    if (gState->motorEnabled) {
      appLogicApplyMotorState();
    }
  }
}
