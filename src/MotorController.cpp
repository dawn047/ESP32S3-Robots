#include "MotorController.h"

// MotorController 类的实现
MotorController::MotorController() 
    : motorL(42, 41, ENCODER_LEFT_A, ENCODER_LEFT_B),    // 左电机(AIN1, AIN2, ENC_A, ENC_B)
      motorR(38, 40, ENCODER_RIGHT_A, ENCODER_RIGHT_B)   // 右电机(BIN1, BIN2, ENC_A, ENC_B)
{
    currentSpeed = 0;    // 初始速度为0
}

void MotorController::begin() {
    motorL.init();
    motorR.init();
}

void MotorController::moveForward() {   // 前进
    motorL.forward(currentSpeed);
    motorR.forward(currentSpeed);
}

void MotorController::moveBackward() {  // 后退
    motorL.backward(currentSpeed);
    motorR.backward(currentSpeed);
}

void MotorController::turnLeft() {  // 左转
    motorL.backward(currentSpeed);
    motorR.forward(currentSpeed);
}

void MotorController::turnRight() { // 右转
    motorL.forward(currentSpeed);
    motorR.backward(currentSpeed);
}

void MotorController::stop() {  // 停止
    motorL.stop();
    motorR.stop();
}

void MotorController::setSpeed(int speed) {   
    currentSpeed = speed;
    motorL.setSpeed(speed);
    motorR.setSpeed(speed);
}

void MotorController::setSpeedOpenLoop(int speedL, int speedR) {    
    motorL.setSpeed(speedL+5);// 左电机速度补偿
    motorR.setSpeed(speedR);// 右电机速度补偿
}

void MotorController::updateEncoders() {
    motorL.updateSpeed();
    motorR.updateSpeed();
    // 计算平均速度（米/秒）
    averageSpeed = (motorL.getCurrentSpeed() + motorR.getCurrentSpeed()) / 2.0;
}

float MotorController::getAverageSpeed() {
    return averageSpeed;
}

float MotorController::getLeftSpeed() {
    return motorL.getCurrentSpeed();
}

float MotorController::getRightSpeed() {
    return motorR.getCurrentSpeed();
}