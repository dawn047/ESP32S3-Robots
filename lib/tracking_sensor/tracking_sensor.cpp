#include "tracking_sensor.h"

void TrackingSensor::init() {
    for (auto pin : sensorPins) {
        pinMode(pin, INPUT_PULLUP);
    }
    Serial.println("Tracking sensors initialized");
}

uint8_t TrackingSensor::readSensors() {
    uint8_t result = 0;
    for (int i = 0; i < 4; i++) {
        sensorStates[i] =!digitalRead(sensorPins[i]); // 不取反是因为传感器遇黑线输出低电平，取反：sensorStates[i] = !digitalRead(sensorPins[i]);
        result |= (sensorStates[i] << i);
    }

    // 滑动平均滤波
    sensorHistory[filterIndex] = result;
    filterIndex = (filterIndex + 1) % filterSize;

    uint8_t filteredResult = 0;
    for (int i = 0; i < filterSize; i++) {
        filteredResult |= sensorHistory[i];
    }

    return filteredResult;
}

void TrackingSensor::printSensorStates() {
    Serial.print("Sensors: ");
    for (bool state : sensorStates) {
        Serial.print(state ? "■ " : "□ ");
    }
    Serial.println();
}