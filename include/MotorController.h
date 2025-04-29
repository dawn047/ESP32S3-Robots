#ifndef MotorController_h
#define MotorController_h

#include <Arduino.h>
#include "Motor.h"

// 定义编码器引脚
#define ENCODER_LEFT_A  18  // 左电机编码器A相
#define ENCODER_LEFT_B   8  // 左电机编码器B相
#define ENCODER_RIGHT_A  3  // 右电机编码器A相
#define ENCODER_RIGHT_B 46  // 右电机编码器B相

class MotorController {
public:
    MotorController();
    void begin();
    void moveForward();
    void moveBackward();
    void turnLeft();
    void turnRight();
    void stop();
  //void brake();
    void setSpeed(int speed);
    void setSpeedOpenLoop(int speedL, int speedR);
    void updateEncoders();  // 更新编码器数据
    float getAverageSpeed(); // 获取平均速度
    float getLeftSpeed();    // 获取左轮速度
    float getRightSpeed();   // 获取右轮速度

private:
    Motor motorL;
    Motor motorR;
    int currentSpeed = 0;
    float averageSpeed = 0;  // 存储平均速度
};

#endif