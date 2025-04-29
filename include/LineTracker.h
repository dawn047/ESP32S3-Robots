#ifndef LINE_TRACKER_H
#define LINE_TRACKER_H

#include "MotorController.h"

class LineTracker {
public:
    static void handleTracking(uint8_t sensorValue, MotorController& motor);
    
private:
    static unsigned long lastLineDetectedTime;  // 上次检测到线的时间
    static bool isRunning;                      // 当前是否在运行状态
    static int lastLeftSpeed;                   // 记录上次左轮速度
    static int lastRightSpeed;                  // 记录上次右轮速度
    static const unsigned long TIMEOUT = 1000;  // 超时时间1秒
};

#endif
