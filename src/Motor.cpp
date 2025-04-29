#include "Motor.h"

Motor::Motor(int in1, int in2, int encA, int encB) 
    : _in1(in1), _in2(in2), _encoder(encA, encB)
{
    _currentPWM = 0;
    _currentSpeed = 0;  // 初始化当前速度
}

void Motor::init() {
    pinMode(_in1, OUTPUT);  
    pinMode(_in2, OUTPUT);
    _encoder.begin();
    stop();
}

void Motor::forward(int speed) {
    _currentSpeed = speed;
    analogWrite(_in1, speed);
    analogWrite(_in2, 0);
}

void Motor::backward(int speed) {
    _currentSpeed = speed;
    analogWrite(_in1, 0);
    analogWrite(_in2, speed);
}

void Motor::stop() {
    _currentSpeed = 0;
    // 使输出端呈高阻态，模拟滑行/休眠状态
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);
    pinMode(_in1, INPUT); 
    pinMode(_in2, INPUT); 
}

/*void Motor::brake() {
    _currentSpeed = 0;
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);
}*/

void Motor::setSpeed(int speed) {
    _currentSpeed = speed;
    if (speed > 0) {
        forward(speed);
    } else if (speed < 0) {
        backward(-speed);
    } else {
        stop();
    }
}

void Motor::updateSpeed() {
    _encoder.update();
}

float Motor::getCurrentRPM() {
    return _encoder.getSpeed();
}

float Motor::getCurrentSpeed() {
    return _encoder.getLinearSpeed();
}

/*AT8236功能逻辑表
（H=高电平，L=低电平，Z=高阻态）


输入状态	输出状态	功能模式
​IN1=0, IN2=0	OUT1=Z, OUT2=Z	​滑行/休眠：输出端呈高阻态，电机自由停止，芯片进入低功耗状态
​IN1=1, IN2=0	OUT1=H, OUT2=L	​正向驱动：OUT1输出高电平，OUT2输出低电平，驱动电机正向运转
​IN1=0, IN2=1	OUT1=L, OUT2=H	​反向驱动：OUT1输出低电平，OUT2输出高电平，驱动电机反向运转
​IN1=1, IN2=1	OUT1=L, OUT2=L	​刹车制动：两端同步输出低电平，电机快速制动停止*/