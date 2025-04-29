#include "Encoder.h"

Encoder* Encoder::instances[2] = {nullptr, nullptr};
uint8_t Encoder::instanceCount = 0;

Encoder::Encoder(int pinA, int pinB) : _pinA(pinA), _pinB(pinB) {
    if (instanceCount < 2) {
        instances[instanceCount++] = this;
    }
}

void Encoder::begin() {
    pinMode(_pinA, INPUT_PULLUP);
    pinMode(_pinB, INPUT_PULLUP);
    
    if (this == instances[0]) {
        attachInterrupt(digitalPinToInterrupt(_pinA), isrA, CHANGE);
    } else if (this == instances[1]) {
        attachInterrupt(digitalPinToInterrupt(_pinA), isrB, CHANGE);
    }
}

void IRAM_ATTR Encoder::isrA() {
    if (instances[0]) {
        if (digitalRead(instances[0]->_pinB) == digitalRead(instances[0]->_pinA)) {
            instances[0]->_count--;
        } else {
            instances[0]->_count++;
        }
    }
}

void IRAM_ATTR Encoder::isrB() {
    if (instances[1]) {
        if (digitalRead(instances[1]->_pinB) == digitalRead(instances[1]->_pinA)) {
            instances[1]->_count--;
        } else {
            instances[1]->_count++;
        }
    }
}

long Encoder::getCount() {
    return _count;
}

void Encoder::reset() {
    _count = 0;
    _lastCount = 0;
}

void Encoder::update() {
    unsigned long currentTime = millis();
    if (currentTime - _lastTime >= 50) {  // 每50ms更新一次
        long deltaCounts = _count - _lastCount;

        _currentRPM = -(float)(deltaCounts * 60000.0) / (_lastTime * PPR * GEAR_RATIO);//添加负号表示反转方向
        
        _lastTime = currentTime;
        _lastCount = _count;
    }
}

float Encoder::getSpeed() {
    return _currentRPM;
}

float Encoder::getLinearSpeed() {
    return PI * WHEEL_DIAMETER * _currentRPM / 60.0;
}
