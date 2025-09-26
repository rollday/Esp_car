#include "buttons.h"

// 按键引脚定义 - 根据实际连接修改（与原 main.cpp 保持一致）
#define K1_PIN GPIO_NUM_15
#define K2_PIN GPIO_NUM_16
#define K3_PIN GPIO_NUM_17
#define K4_PIN GPIO_NUM_18

struct Button
{
    uint8_t pin;
    bool lastState;
    bool currentState;
    bool stableState;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
    bool isPressedFlag;
    bool longPressTriggered;
    bool shortPressHandled;
};

static Button buttons[4];
static unsigned long lastCheckTime = 0;
static const unsigned long CHECK_INTERVAL = 10;
static bool systemStable = false;
static unsigned long systemStartTime = 0;

static ButtonCallback shortCb = nullptr;
static ButtonCallback longCb = nullptr;

void buttonsSetShortPressHandler(ButtonCallback cb) { shortCb = cb; }
void buttonsSetLongPressHandler(ButtonCallback cb) { longCb = cb; }

void buttonsInit()
{
    Button btn1 = {K1_PIN, HIGH, HIGH, HIGH, 0, 0, false, false, false};
    Button btn2 = {K2_PIN, HIGH, HIGH, HIGH, 0, 0, false, false, false};
    Button btn3 = {K3_PIN, HIGH, HIGH, HIGH, 0, 0, false, false, false};
    Button btn4 = {K4_PIN, HIGH, HIGH, HIGH, 0, 0, false, false, false};

    buttons[0] = btn1;
    buttons[1] = btn2;
    buttons[2] = btn3;
    buttons[3] = btn4;

    for (int i = 0; i < 4; i++)
    {
        pinMode(buttons[i].pin, INPUT_PULLUP);
    }

    delay(50);
    for (int i = 0; i < 4; i++)
    {
        bool initialState = digitalRead(buttons[i].pin);
        buttons[i].lastState = initialState;
        buttons[i].stableState = initialState;
        Serial.print("按键");
        Serial.print(i + 1);
        Serial.print(" 初始状态: ");
        Serial.println(initialState == HIGH ? "HIGH" : "LOW");
    }

    systemStartTime = millis();
    systemStable = false;
    lastCheckTime = 0;
}

void buttonsPoll()
{
    unsigned long currentTime = millis();

    // 系统启动后忽略前500ms按键事件
    if (!systemStable && (currentTime - systemStartTime < 500))
    {
        return;
    }
    systemStable = true;

    if (currentTime - lastCheckTime < CHECK_INTERVAL)
    {
        return;
    }
    lastCheckTime = currentTime;

    for (int i = 0; i < 4; i++)
    {
        bool reading = digitalRead(buttons[i].pin);

        if (reading != buttons[i].lastState)
        {
            buttons[i].lastDebounceTime = currentTime;
            Serial.print("按键");
            Serial.print(i + 1);
            Serial.print(" 状态变化: ");
            Serial.println(reading == HIGH ? "HIGH" : "LOW");
        }

        if ((currentTime - buttons[i].lastDebounceTime) > 20)
        {
            if (reading != buttons[i].stableState)
            {
                buttons[i].stableState = reading;

                if (buttons[i].stableState == LOW)
                {
                    buttons[i].pressStartTime = currentTime;
                    buttons[i].isPressedFlag = true;
                    buttons[i].longPressTriggered = false;
                    buttons[i].shortPressHandled = false;
                    Serial.print("按键");
                    Serial.print(i + 1);
                    Serial.println(" 按下");
                }
                else
                {
                    buttons[i].isPressedFlag = false;
                    buttons[i].longPressTriggered = false;
                    Serial.print("按键");
                    Serial.print(i + 1);
                    Serial.println(" 释放");
                }
            }
        }

        buttons[i].lastState = reading;

        // 长按检测（1秒以上）
        if (buttons[i].isPressedFlag && !buttons[i].longPressTriggered)
        {
            if (currentTime - buttons[i].pressStartTime > 1000 && buttons[i].stableState == LOW)
            {
                buttons[i].longPressTriggered = true;
                if (longCb)
                    longCb(i);
                buttons[i].shortPressHandled = true; // 避免长按后触发短按
            }
        }
    }

    // 短按事件派发
    if (!systemStable)
        return;
    for (int i = 0; i < 4; i++)
    {
        if (!buttons[i].isPressedFlag && buttons[i].lastState == HIGH &&
            buttons[i].stableState == HIGH && !buttons[i].shortPressHandled &&
            millis() - buttons[i].lastDebounceTime < 500)
        {
            if (shortCb)
                shortCb(i);
            buttons[i].shortPressHandled = true;
        }
    }
}
