#include "LineTracker.h"

// 静态成员初始化
unsigned long LineTracker::lastLineDetectedTime = 0;
bool LineTracker::isRunning = false;
int LineTracker::lastLeftSpeed = 0;
int LineTracker::lastRightSpeed = 0;

void LineTracker::handleTracking(uint8_t sensorValue, MotorController& motor) {
    const int baseSpeed = 70;      // 基础速度
    const int turnSpeed = 60;      // 转弯速度
    const int slowSpeed = 50;      // 慢速
    
    if (sensorValue == 0b0000) {
        // 检测不到线时的处理
        if (isRunning) {
            // 如果之前在运行，检查是否超时
            if (millis() - lastLineDetectedTime > TIMEOUT) {
                motor.stop();
                isRunning = false;
            } else {
                // 继续保持上一次的速度
                motor.setSpeedOpenLoop(lastLeftSpeed, lastRightSpeed);
            }
        }
        return;
    }

    // 检测到线，更新时间戳
    lastLineDetectedTime = millis();
    isRunning = true;

    // 记录当前速度，用于丢线时继续运动
    switch(sensorValue) {
        case 0b0110:
            lastLeftSpeed = baseSpeed;
            lastRightSpeed = baseSpeed;
            motor.setSpeedOpenLoop(baseSpeed, baseSpeed);
            break;
        
        // 小偏移情况
        case 0b0100:
            lastLeftSpeed = slowSpeed;
            lastRightSpeed = baseSpeed;
            motor.setSpeedOpenLoop(slowSpeed, baseSpeed);
            break;
        case 0b0010:
            lastLeftSpeed = baseSpeed;
            lastRightSpeed = slowSpeed;
            motor.setSpeedOpenLoop(baseSpeed, slowSpeed);
            break;
            
        // 大偏移情况
        case 0b1000:
            lastLeftSpeed = turnSpeed;
            lastRightSpeed = -turnSpeed;
            motor.setSpeedOpenLoop(turnSpeed, -turnSpeed);
            break;
        case 0b0001:
            lastLeftSpeed = -turnSpeed;
            lastRightSpeed = turnSpeed;
            motor.setSpeedOpenLoop(-turnSpeed, turnSpeed);
            break;
            
        // 大角度转弯情况
        case 0b1100:
            lastLeftSpeed = baseSpeed;
            lastRightSpeed = -slowSpeed;
            motor.setSpeedOpenLoop(baseSpeed, -slowSpeed);
            break;
        case 0b0011:
            lastLeftSpeed = -slowSpeed;
            lastRightSpeed = baseSpeed;
            motor.setSpeedOpenLoop(-slowSpeed, baseSpeed);
            break;
            
        // 特殊情况处理
        case 0b1111:    // 可能是十字路口
            lastLeftSpeed = slowSpeed;
            lastRightSpeed = slowSpeed;
            motor.setSpeedOpenLoop(slowSpeed, slowSpeed);
            break;
        
        // 其他未定义状态，停止以确保安全
        default:
            motor.stop();
            isRunning = false;
            break;
    }
}
