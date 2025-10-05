#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include "display.h"

Adafruit_MPU6050 mpu;

// 传感器数据变量
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;

// 姿态变量
float roll = 0, pitch = 0, yaw = 0;

// 速度变量
float velocityX = 0, velocityY = 0;

// 时间变量
unsigned long lastTime = 0;
float deltaTime = 0;
static constexpr unsigned long DISPLAY_INTERVAL_MS = 200;
unsigned long lastDisplayUpdateMs = 0;

// 校准变量
float gyroXoffset = 0, gyroYoffset = 0, gyroZoffset = 0;
float accelXoffset = 0, accelYoffset = 0, accelZoffset = 0;
const float GRAVITY = 9.80665f;
bool calibrated = false;

// 定义MPU6050引脚
#define MPU6050_SDA 5  // 与 OLED 共用 I2C 总线
#define MPU6050_SCL 4  // 与 OLED 共用 I2C 总线

// 偏航角静止检测阈值
#define YAW_ZERO_THRESHOLD 0.1 // deg/s
#define YAW_ZERO_TIME 2000     // ms

// 偏航角静止检测变量
unsigned long yawZeroStartTime = 0;
bool yawZeroing = false;

// 函数声明
void calibrateGyro();
void calibrateAccelerometer();
void readMPU6050();
void updateAttitude();
void calculateVelocity();
void setup()
{
  Serial.begin(115200);

  // 设置I2C引脚
  Wire.begin(MPU6050_SDA, MPU6050_SCL);

  // 初始化MPU6050
  if (!mpu.begin())
  {
    Serial.println("找不到MPU6050芯片！");
    while (1)
    {
      delay(10);
    }
  }
  Serial.println("MPU6050初始化成功！");

  // 配置传感器范围
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // 校准陀螺仪
  calibrateGyro();
  calibrateAccelerometer();

  // 初始化显示
  if (!initDisplay())
  {
    Serial.println("OLED初始化失败");
  }
  else
  {
    Serial.println("OLED初始化成功");
    updateAttitudeDisplay(roll, pitch, yaw, velocityX, velocityY);
  }

  lastTime = micros();
}

void loop()
{
  // 获取时间差
  unsigned long currentTime = micros();
  deltaTime = (currentTime - lastTime) / 1000000.0; // 转换为秒
  lastTime = currentTime;

  // 读取传感器数据
  readMPU6050();

  // 更新姿态
  updateAttitude();

  // 计算速度
  calculateVelocity();

  if (millis() - lastDisplayUpdateMs >= DISPLAY_INTERVAL_MS)
  {
    lastDisplayUpdateMs = millis();
    updateAttitudeDisplay(roll, pitch, yaw, velocityX, velocityY);
  }

  delay(5);
}

void readMPU6050()
{
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 读取加速度计数据（m/s²）
  accelX = a.acceleration.x - accelXoffset;
  accelY = a.acceleration.y - accelYoffset;
  accelZ = a.acceleration.z - accelZoffset;

  // 读取陀螺仪数据并应用校准（deg/s）
  gyroX = g.gyro.x * 180 / PI - gyroXoffset;
  gyroY = g.gyro.y * 180 / PI - gyroYoffset;
  gyroZ = g.gyro.z * 180 / PI - gyroZoffset;
}

void calibrateGyro()
{
  Serial.println("校准陀螺仪，保持传感器静止...");
  float sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < 1000; i++)
  {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    sumX += g.gyro.x * 180 / PI;
    sumY += g.gyro.y * 180 / PI;
    sumZ += g.gyro.z * 180 / PI;
    delay(5);
  }

  gyroXoffset = sumX / 1000;
  gyroYoffset = sumY / 1000;
  gyroZoffset = sumZ / 1000;

  calibrated = true;
  Serial.println("校准完成");
}

void calibrateAccelerometer()
{
  Serial.println("校准加速度计，保持传感器静止并水平...");
  const int samples = 1000;
  float sumX = 0, sumY = 0, sumZ = 0;
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

void updateAttitude()
{
  // 互补滤波计算姿态
  float accelRoll = atan2(accelY, accelZ) * 180 / PI;
  float accelPitch = atan2(-accelX, sqrt(accelY * accelY + accelZ * accelZ)) * 180 / PI;

  // 陀螺仪积分
  roll = 0.98 * (roll + gyroX * deltaTime) + 0.02 * accelRoll;
  pitch = 0.98 * (pitch + gyroY * deltaTime) + 0.02 * accelPitch;
  yaw += gyroZ * deltaTime;

  // 偏航角静止检测
  if (fabs(gyroZ) < YAW_ZERO_THRESHOLD)
  {
    if (!yawZeroing)
    {
      yawZeroing = true;
      yawZeroStartTime = millis();
    }
    else if (millis() - yawZeroStartTime >= YAW_ZERO_TIME)
    {
      yaw = 0; // 自动归零
      Serial.println("偏航角已自动归零");
      yawZeroing = false;
    }
  }
  else
  {
    yawZeroing = false;
  }
}

void calculateVelocity()
{
  float rollRad = roll * PI / 180.0f;
  float pitchRad = pitch * PI / 180.0f;

  float sinRoll = sin(rollRad);
  float cosRoll = cos(rollRad);
  float sinPitch = sin(pitchRad);
  float cosPitch = cos(pitchRad);

  float gravityX = -sinPitch * GRAVITY;
  float gravityY = sinRoll * cosPitch * GRAVITY;

  float linearAccelX = accelX - gravityX;
  float linearAccelY = accelY - gravityY;

  if (fabs(linearAccelX) < 0.05f) linearAccelX = 0;
  if (fabs(linearAccelY) < 0.05f) linearAccelY = 0;

  velocityX += linearAccelX * deltaTime;
  velocityY += linearAccelY * deltaTime;

  velocityX *= 0.99;
  velocityY *= 0.99;
}
