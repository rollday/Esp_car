#pragma once

void initMotors();

void motorA(int speed);
void motorB(int speed);
void motors(int speedA, int speedB);

void brakeAll();
void standby();
void wakeup();

int getSpeedA();
int getSpeedB();
