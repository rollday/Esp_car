#include <Arduino.h>
#include "motors.h"
#include "display.h"

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
}

void loop()
{
  static bool started = false;
  static uint32_t lastUpdate = 0;

  if (!started)
  {
    motors(0, 255);
    started = true;
  }

  if (millis() - lastUpdate >= 200)
  {
    updateDisplay(getSpeedA(), getSpeedB());
    lastUpdate = millis();
  }
}