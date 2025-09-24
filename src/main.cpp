#include <Arduino.h>
// TB6612FNG 电机控制引脚定义
// 电机A
const int AIN1 = 9;  // 方向引脚1
const int AIN2 = 10; // 方向引脚2
const int PWMA = 8;  // PWM速度控制

// 电机B
const int BIN1 = 12; // 方向引脚1
const int BIN2 = 13; // 方向引脚2
const int PWMB = 11; // PWM速度控制

// 公共引脚
const int STBY = 14; // 待机控制

// PWM参数
const int pwmChannelA = 0; // 电机A的PWM通道
const int pwmChannelB = 1; // 电机B的PWM通道
const int freq = 1000;     // PWM频率1kHz
const int resolution = 8;  // 8位分辨率(0-255)

void setup()
{
  Serial.begin(115200);

  // 设置引脚模式
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(STBY, OUTPUT);

  // 配置PWM
  ledcSetup(pwmChannelA, freq, resolution);
  ledcSetup(pwmChannelB, freq, resolution);
  ledcAttachPin(PWMA, pwmChannelA);
  ledcAttachPin(PWMB, pwmChannelB);

  // 启动驱动芯片（取消待机）
  digitalWrite(STBY, HIGH);

  Serial.println("TB6612FNG 电机驱动初始化完成");
}

// 控制单个电机函数
void motorA(int speed)
{
  speed = constrain(speed, -255, 255); // 限制速度范围

  if (speed > 0)
  {
    // 正转
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    ledcWrite(pwmChannelA, speed);
  }
  else if (speed < 0)
  {
    // 反转
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    ledcWrite(pwmChannelA, -speed);
  }
  else
  {
    // 停止（刹车）
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, HIGH);
    ledcWrite(pwmChannelA, 0);
  }
}

void motorB(int speed)
{
  speed = constrain(speed, -255, 255);

  if (speed > 0)
  {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    ledcWrite(pwmChannelB, speed);
  }
  else if (speed < 0)
  {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    ledcWrite(pwmChannelB, -speed);
  }
  else
  {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, HIGH);
    ledcWrite(pwmChannelB, 0);
  }
}

// 同时控制两个电机
void motors(int speedA, int speedB)
{
  motorA(speedA);
  motorB(speedB);
}

// 刹车函数
void brakeAll()
{
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, HIGH);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, HIGH);
  ledcWrite(pwmChannelA, 0);
  ledcWrite(pwmChannelB, 0);
}

// 进入待机模式（低功耗）
void standby()
{
  digitalWrite(STBY, LOW);
}

// 退出待机模式
void wakeup()
{
  digitalWrite(STBY, HIGH);
}

void loop()
{
  Serial.println("=== 电机测试开始 ===");

  // 电机A正转测试
  Serial.println("电机A正转加速");
  for (int speed = 0; speed <= 255; speed += 10)
  {
    motorA(speed);
    Serial.print("电机A速度: ");
    Serial.println(speed);
    delay(100);
  }
  delay(1000);

  // 电机A反转测试
  Serial.println("电机A反转");
  for (int speed = 0; speed >= -255; speed -= 10)
  {
    motorA(speed);
    Serial.print("电机A速度: ");
    Serial.println(speed);
    delay(100);
  }
  delay(1000);

  // 电机B测试
  Serial.println("电机B正转");
  for (int speed = 0; speed <= 255; speed += 10)
  {
    motorB(speed);
    Serial.print("电机B速度: ");
    Serial.println(speed);
    delay(100);
  }
  delay(1000);

  // 双电机协同控制
  Serial.println("双电机前进");
  motors(200, 200);
  delay(2000);

  Serial.println("左转");
  motors(100, 200);
  delay(1000);

  Serial.println("右转");
  motors(200, 100);
  delay(1000);

  Serial.println("刹车");
  brakeAll();
  delay(2000);

  Serial.println("待机模式");
  standby();
  delay(2000);

  Serial.println("唤醒");
  wakeup();
  delay(1000);
}