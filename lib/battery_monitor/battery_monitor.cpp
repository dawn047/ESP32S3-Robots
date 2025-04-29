#include "battery_monitor.h"

//构造函数，初始化ADC引脚、分压比和参考电压
BatteryMonitor::BatteryMonitor(int adcPin, float voltageDividerRatio, float adcRefVoltage)
    : _adcPin(adcPin),
      _voltageDividerRatio(voltageDividerRatio),
      _adcRefVoltage(adcRefVoltage),
      _rawValue(0),
      _batteryVoltage(0.0f),
      _batteryLevel(0) {}

//构造函数，初始化ADC引脚、分压比和参考电压
void BatteryMonitor::begin() {
    analogReadResolution(12);
    analogSetPinAttenuation(_adcPin, ADC_11db);
}

//初始化ADC引脚，设置分辨率为12位，衰减为11dB（适用于3.3V参考电压）
void BatteryMonitor::update() {
    _rawValue = _readFilteredADC();
    float adcVoltage = (_rawValue / 4095.0f) * _adcRefVoltage;
    _batteryVoltage = adcVoltage * _voltageDividerRatio;
    _batteryLevel = _mapVoltageToLevel(_batteryVoltage);
}

//读取ADC值并进行滤波，返回平均值
int BatteryMonitor::_readFilteredADC() {
    int raw = 0;
    for (int i = 0; i < 16; i++) {
        raw += analogRead(_adcPin);
        delay(1);
    }
    return raw / 16;
}

//基于常见电池电压水平的简单线性映射，将电压映射为电池电量百分比（0 - 100%）
int BatteryMonitor::_mapVoltageToLevel(float voltage) const {
    if (voltage >= 12.6) return 100;
    if (voltage >= 12.1) return 80;
    if (voltage >= 11.8) return 60;
    if (voltage >= 11.5) return 40;
    if (voltage >= 11.0) return 20;
    return 0;
}
//获取电池电压、电量和原始ADC值
float BatteryMonitor::getVoltage() const { return _batteryVoltage; }
//获取电池电量百分比
int BatteryMonitor::getLevel() const { return _batteryLevel; }
//获取原始ADC值
int BatteryMonitor::getRawValue() const { return _rawValue; }
//设置分压比和参考电压
void BatteryMonitor::setVoltageDividerRatio(float ratio) { _voltageDividerRatio = ratio; }
//设置参考电压
void BatteryMonitor::setAdcRefVoltage(float voltage) { _adcRefVoltage = voltage; }