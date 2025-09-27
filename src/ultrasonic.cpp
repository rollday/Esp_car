#include "ultrasonic.h"

// 固定引脚（可按需修改）
#define ULTRASONIC_TRIG 20
#define ULTRASONIC_ECHO 21

static bool ultrasonicInited = false;

bool initUltrasonic()
{
    pinMode(ULTRASONIC_TRIG, OUTPUT);
    pinMode(ULTRASONIC_ECHO, INPUT);
    digitalWrite(ULTRASONIC_TRIG, LOW);
    ultrasonicInited = true;
    return true;
}

float ultrasonicReadCm()
{
    if (!ultrasonicInited)
        return -1.0f;

    // 触发 10us 脉冲
    digitalWrite(ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG, LOW);

    // 读取回波，30ms 超时（约 5m）
    unsigned long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000UL);
    if (duration == 0)
        return -1.0f;

    // 距离(cm) = (us * 17) / 1000
    return (duration * 17.0f) / 1000.0f;
}
