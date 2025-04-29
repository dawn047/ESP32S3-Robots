// Ultrasonic.h 
#ifndef ULTRASONIC_H 
#define ULTRASONIC_H 
 
#include <Arduino.h>

// 超声波配置
const uint8_t trigPin = 15; // 超声波触发引脚,GPIO15-trig
const uint8_t echoPin = 16;  // 超声波接收引脚,GPIO16-echo

class Ultrasonic {
public:
    Ultrasonic(uint8_t trigPin, uint8_t echoPin);
    float getDistanceCM();
    
private:
    uint8_t _trigPin;
    uint8_t _echoPin;
    unsigned long pulseTimeout = 30000; // 超时时间(us)
};
 
#endif 