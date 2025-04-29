// DisplayModule.h
#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// OLED配置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_I2C_ADDR 0x3C
#define OLED_SDA 48 // OLED的SDA引脚，GPIO48-SDA
#define OLED_SCL 47 // OLED的SCL引脚，GPIO47-SCL

class DisplayModule {
private:
    Adafruit_SSD1306 display;
    uint8_t sdaPin;
    uint8_t sclPin;
    uint8_t i2cAddress;
    int screenWidth;
    int screenHeight;
    bool bleIconVisible;  // 蓝牙图标是否可见
    unsigned long lastBleBlinkTime;  // 上次蓝牙图标闪烁时间
    void drawBattery(int x, int y, int percentage);
    void drawDirectionArrow(int direction);
    void drawBleIcon(int x, int y, bool connected);  // 新增蓝牙图标绘制函数

public:
    DisplayModule(uint8_t sda, uint8_t scl, uint8_t addr, int width, int height);
    bool init();
    void showDistance(float distance);
    void showSystemStatus(const String& status);
    void showBootAnimation();
    enum Direction {
        NONE = 0,
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };
    void updateDisplayData(float distance, int batteryLevel, const String& status, 
                         int direction, bool bleConnected, const String& roomInfo = "");  // 添加roomInfo参数
    void clearSection(int x, int y, int w, int h);
};