#include "mpu.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <math.h>

namespace
{
Adafruit_MPU6050 mpu;
float accelXoffset = 0.0f, accelYoffset = 0.0f, accelZoffset = 0.0f;
float gyroXoffset = 0.0f, gyroYoffset = 0.0f, gyroZoffset = 0.0f;

constexpr float GRAVITY = 9.80665f;
constexpr float YAW_ZERO_THRESHOLD = 0.1f;
constexpr unsigned long YAW_ZERO_TIME = 2000;

MpuState state = {};
bool yawZeroing = false;
unsigned long yawZeroStartTime = 0;
bool initialized = false;

void calibrateGyro()
{
  Serial.println("校准陀螺仪，保持传感器静止...");
  float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;

  for (int i = 0; i < 1000; ++i)
  {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumX += g.gyro.x * 180.0f / PI;
    sumY += g.gyro.y * 180.0f / PI;
    sumZ += g.gyro.z * 180.0f / PI;
    delay(5);
  }

  gyroXoffset = sumX / 1000.0f;
  gyroYoffset = sumY / 1000.0f;
  gyroZoffset = sumZ / 1000.0f;

  Serial.println("校准完成");
}

void calibrateAccelerometer()
{
  Serial.println("校准加速度计，保持传感器静止并水平...");
  const int samples = 1000;
  float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;
  for (int i = 0; i < samples; ++i)
  {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumX += a.acceleration.x;
    sumY += a.acceleration.y;
    sumZ += a.acceleration.z;
    delay(5);
  }
  accelXoffset = sumX / samples;
  accelYoffset = sumY / samples;
  accelZoffset = sumZ / samples - GRAVITY;
  Serial.println("加速度计校准完成");
}
} // namespace

bool mpuInit(uint8_t sdaPin, uint8_t sclPin)
{
  Wire.begin(sdaPin, sclPin);

  if (!mpu.begin())
  {
    Serial.println("找不到MPU6050芯片！");
    return false;
  }
  Serial.println("MPU6050初始化成功！");

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  calibrateGyro();
  calibrateAccelerometer();

  state = {};
  yawZeroing = false;
  yawZeroStartTime = 0;
  initialized = true;
  return true;
}

void mpuUpdate(float deltaTime)
{
  if (!initialized)
  {
    return;
  }

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  const float accelX = a.acceleration.x - accelXoffset;
  const float accelY = a.acceleration.y - accelYoffset;
  const float accelZ = a.acceleration.z - accelZoffset;

  const float gyroX = g.gyro.x * 180.0f / PI - gyroXoffset;
  const float gyroY = g.gyro.y * 180.0f / PI - gyroYoffset;
  const float gyroZ = g.gyro.z * 180.0f / PI - gyroZoffset;

  const float accelRoll = atan2f(accelY, accelZ) * 180.0f / PI;
  const float accelPitch = atan2f(-accelX, sqrtf(accelY * accelY + accelZ * accelZ)) * 180.0f / PI;

  state.roll = 0.98f * (state.roll + gyroX * deltaTime) + 0.02f * accelRoll;
  state.pitch = 0.98f * (state.pitch + gyroY * deltaTime) + 0.02f * accelPitch;
  state.yaw += gyroZ * deltaTime;

  if (fabsf(gyroZ) < YAW_ZERO_THRESHOLD)
  {
    if (!yawZeroing)
    {
      yawZeroing = true;
      yawZeroStartTime = millis();
    }
    else if (millis() - yawZeroStartTime >= YAW_ZERO_TIME)
    {
      state.yaw = 0.0f;
      Serial.println("偏航角已自动归零");
      yawZeroing = false;
    }
  }
  else
  {
    yawZeroing = false;
  }

  const float rollRad = state.roll * PI / 180.0f;
  const float pitchRad = state.pitch * PI / 180.0f;

  const float sinRoll = sinf(rollRad);
  const float cosRoll = cosf(rollRad);
  const float sinPitch = sinf(pitchRad);
  const float cosPitch = cosf(pitchRad);

  const float gravityX = -sinPitch * GRAVITY;
  const float gravityY = sinRoll * cosPitch * GRAVITY;

  float linearAccelX = accelX - gravityX;
  float linearAccelY = accelY - gravityY;

  if (fabsf(linearAccelX) < 0.05f)
    linearAccelX = 0.0f;
  if (fabsf(linearAccelY) < 0.05f)
    linearAccelY = 0.0f;

  state.velocityX += linearAccelX * deltaTime;
  state.velocityY += linearAccelY * deltaTime;

  state.velocityX *= 0.99f;
  state.velocityY *= 0.99f;
}

const MpuState &mpuGetState()
{
  return state;
}
