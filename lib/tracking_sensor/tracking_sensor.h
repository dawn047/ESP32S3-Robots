#ifndef TRACKING_SENSOR_H
#define TRACKING_SENSOR_H

#include <Arduino.h>

// 定义传感器引脚
#define SENSOR1_PIN 4
#define SENSOR2_PIN 5
#define SENSOR3_PIN 6
#define SENSOR4_PIN 7

class TrackingSensor {
public:
    void init();// 初始化传感器引脚
    void printSensorStates();// 打印传感器状态
    uint8_t readSensors(); // 读取传感器状态并进行滤波

private:
    const uint8_t sensorPins[4] = {SENSOR1_PIN, SENSOR2_PIN, SENSOR3_PIN, SENSOR4_PIN};
    bool sensorStates[4] = {0};
    const int filterSize = 5;  // 滤波窗口大小
    int filterIndex = 0;
    uint8_t sensorHistory[5];
};

#endif