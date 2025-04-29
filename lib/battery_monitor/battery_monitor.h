#ifndef BATTERY_MONITOR_H
#define BATTERY_MONITOR_H

#include <Arduino.h>

class BatteryMonitor {
public:
    BatteryMonitor(int adcPin = 1, // ADC引脚，默认1号引脚  
                  float voltageDividerRatio = 11.96f,   // 分压比,理论值11：1，实际值11.96，电阻100kΩ：10kΩ
                  float adcRefVoltage = 3.29f); // 参考电压，理论值3.3V，实际值3.29V
    
    void begin();
    void update();
    float getVoltage() const;
    int getLevel() const;
    int getRawValue() const;
    
    void setVoltageDividerRatio(float ratio);
    void setAdcRefVoltage(float voltage);

private:
    const int _adcPin;
    float _voltageDividerRatio;
    float _adcRefVoltage;
    
    int _rawValue;
    float _batteryVoltage;
    int _batteryLevel;
    
    int _readFilteredADC();
    int _mapVoltageToLevel(float voltage) const;
};

#endif