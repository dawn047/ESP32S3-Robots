#ifndef ENCODER_H
#define ENCODER_H

#include <Arduino.h>

class Encoder {
public:
    Encoder(int pinA, int pinB);
    void begin();
    long getCount();
    void reset();
    float getSpeed();  // 返回RPM
    float getLinearSpeed();  // 返回米/秒
    void update();  // 需要定期调用以更新速度

private:
    static void IRAM_ATTR isrA();
    static void IRAM_ATTR isrB();
    
    int _pinA, _pinB;
    volatile long _count = 0;
    unsigned long _lastTime = 0;
    long _lastCount = 0;
    float _currentRPM = 0;
    
    // 修改常量定义
    static const int PPR = 2496;  // 编码器每圈脉冲数
    static const int GEAR_RATIO = 48;  // 减速比
    static constexpr float WHEEL_DIAMETER = 0.065f;  // 轮径(米)
    static constexpr float pi = 3.14159265359f;  // 圆周率
    
    static Encoder* instances[2];  // 支持两个编码器实例
    static uint8_t instanceCount;
};

#endif
