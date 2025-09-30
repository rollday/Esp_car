#pragma once

#include <Arduino.h>

struct MpuState
{
  float roll;
  float pitch;
  float yaw;
  float velocityX;
  float velocityY;
};

bool mpuInit(uint8_t sdaPin, uint8_t sclPin);
void mpuUpdate(float deltaTime);
const MpuState &mpuGetState();
