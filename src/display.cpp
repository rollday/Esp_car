#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED 定义
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define I2C_ADDRESS 0x3C

// 根据原代码的 I2C 引脚
static const int SDA_PIN = 5;
static const int SCL_PIN = 4;

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool initDisplay()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS))
    {
        // 与原逻辑一致，失败时返回 false，由上层处理
        return false;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED Ready");
    display.display();
    return true;
}

void updateDisplay(int currentSpeedA, int currentSpeedB)
{
    const char *dirA = (currentSpeedA > 0) ? "FWD" : (currentSpeedA < 0) ? "REV"
                                                                         : "STOP";
    const char *dirB = (currentSpeedB > 0) ? "FWD" : (currentSpeedB < 0) ? "REV"
                                                                         : "STOP";
    int valA = abs(currentSpeedA);
    int valB = abs(currentSpeedB);

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.printf("A:%s %3d", dirA, valA);
    display.setCursor(0, 32);
    display.printf("B:%s %3d", dirB, valB);
    display.display();
}
