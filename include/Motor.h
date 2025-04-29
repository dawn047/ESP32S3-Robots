#ifndef Motor_h
#define Motor_h

#include <Arduino.h>
#include "Encoder.h"

class Motor {
public:
    Motor(int in1, int in2, int encA, int encB);
    void init();
    void forward(int speed);
    void backward(int speed);
    void stop();
    void setSpeed(int speed);
    
    // 新增的速度相关方法
    float getCurrentRPM();
    float getCurrentSpeed();
    void updateSpeed();

private:
    int _in1, _in2;
    int _currentPWM;
    int _currentSpeed;  // 添加当前速度变量
    Encoder _encoder;
};

#endif