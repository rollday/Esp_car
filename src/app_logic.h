#pragma once

struct AppState {
  bool motorEnabled;
  bool motorForward;
  bool displayEnabled;
  bool obstacleDetected;
};

using ClearDisplayFn = void (*)();

void appLogicInit(AppState &state, ClearDisplayFn clearFn);
void appLogicApplyMotorState();
void appLogicOnShortPress(int buttonIndex);
void appLogicOnLongPress(int buttonIndex);
void appLogicHandleObstacle(float distanceCm);
const AppState &appLogicGetState();
