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
static const int SDA_PIN = 47;
static const int SCL_PIN = 48;

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static bool isDisplayInitialized = false; // 添加一个标志位
static float traveledDistanceMeters = 0.0f;

bool initDisplay()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    if (!display.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS))
    {
        // 与原逻辑一致，失败时返回 false，由上层处理
        return false;
    }

    isDisplayInitialized = true; // 初始化成功后设置标志位
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.display();
    return true;
}

void updateDisplay(int currentSpeedA, int currentSpeedB)
{
    if (!isDisplayInitialized) // 检查是否已初始化
    {
        return;
    }

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

void updateDisplay(float distance)
{
    if (!isDisplayInitialized) // 检查是否已初始化
    {
        return;
    }

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);

    // 显示距离
    display.setCursor(0, 0);
    display.printf("%.1fcm", distance);

    display.display();
}

void updateDisplay(float distance, int currentSpeedA, int currentSpeedB)
{
    if (!isDisplayInitialized)
        return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // 第一行：距离
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.printf("D:%.1fcm", distance);

    // 第二行：两路速度，一行展示
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.printf("V: %4d %4d", currentSpeedA, currentSpeedB);

    display.display();
}

void updateAttitudeDisplay(float roll, float pitch, float yaw, float velocityX, float velocityY)
{
    if (!isDisplayInitialized)
    {
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("R:%5.1f P:%5.1f", roll, pitch);
    display.setCursor(0, 16);
    display.printf("Y:%5.1f", yaw);
    display.setCursor(0, 32);
    display.printf("VX:%5.2f", velocityX);
    display.setCursor(0, 48);
    display.printf("VY:%5.2f", velocityY);
    display.display();
}

void updateDisplay(float distance, bool motorEnabled, bool motorForward, float planarVelocity)
{
    if (!isDisplayInitialized)
    {
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.printf("D:%4.1fcm", distance);

    display.setTextSize(1);
    display.setCursor(0, 32);
    display.printf("Vt:%5.2f", planarVelocity);
    display.setCursor(0, 44);
    display.printf("Motor:%s", motorEnabled ? "ON" : "OFF");
    display.setCursor(0, 56);
    display.printf("Dir:%s", motorForward ? "FWD" : "REV");

    display.display();
}

void updateDisplay(float distance, bool motorEnabled, bool motorForward, float planarVelocity, float yaw)
{
    if (!isDisplayInitialized)
    {
        return;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.printf("D:%4.1fcm", distance);

    display.setTextSize(1);
    display.setCursor(0, 32);
    display.printf("Vt:%5.2f", planarVelocity);
    display.setCursor(0, 44);
    display.printf("Yaw:%6.1f D:%5.2fm", yaw, traveledDistanceMeters);
    display.setCursor(0, 56);
    display.printf("Motor:%s %s", motorEnabled ? "ON " : "OFF", motorForward ? "FWD" : "REV");

    display.display();
}

// 清屏函数实现
void clearDisplay()
{
    if (!isDisplayInitialized) // 检查是否已初始化
    {
        return;
    }

    display.clearDisplay(); // 清除显示内容
    display.display();      // 刷新屏幕，确保黑屏
}

void displaySetTraveledDistance(float distanceMeters)
{
    traveledDistanceMeters = distanceMeters;
}
