#include <Arduino.h>

// TB6612FNG 电机控制引脚定义
// 电机A
static const int AIN1 = 10; // 方向引脚1
static const int AIN2 = 9;  // 方向引脚2
static const int PWMA = 46; // PWM速度控制

// 电机B
static const int BIN1 = 12; // 方向引脚1
static const int BIN2 = 13; // 方向引脚2
static const int PWMB = 14; // PWM速度控制

// 公共引脚
static const int STBY = 11; // 待机控制

// PWM参数
static const int pwmChannelA = 0; // 电机A的PWM通道
static const int pwmChannelB = 1; // 电机B的PWM通道
static const int freq = 1000;     // PWM频率1kHz
static const int resolution = 8;  // 8位分辨率(0-255)

// 记录当前速度（带符号，正=正转，负=反转）
static volatile int currentSpeedA = 0;
static volatile int currentSpeedB = 0;

void initMotors()
{
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(STBY, OUTPUT);

    ledcSetup(pwmChannelA, freq, resolution);
    ledcSetup(pwmChannelB, freq, resolution);
    ledcAttachPin(PWMA, pwmChannelA);
    ledcAttachPin(PWMB, pwmChannelB);

    // 启动驱动芯片（取消待机）
    digitalWrite(STBY, HIGH);
}

void motorA(int speed)
{
    speed = constrain(speed, -255, 255);

    if (speed > 0)
    {
        digitalWrite(AIN1, HIGH);
        digitalWrite(AIN2, LOW);
        ledcWrite(pwmChannelA, speed);
    }
    else if (speed < 0)
    {
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
    currentSpeedA = speed;
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
    currentSpeedB = speed;
}

void motors(int speedA, int speedB)
{
    motorA(speedA);
    motorB(speedB);
}

void brakeAll()
{
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, HIGH);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, HIGH);
    ledcWrite(pwmChannelA, 0);
    ledcWrite(pwmChannelB, 0);
}

void standby()
{
    digitalWrite(STBY, LOW);
}

void wakeup()
{
    digitalWrite(STBY, HIGH);
}

int getSpeedA() { return currentSpeedA; }
int getSpeedB() { return currentSpeedB; }
