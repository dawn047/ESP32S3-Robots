// DisplayModule.cpp
#include "DisplayModule.h"

DisplayModule::DisplayModule(uint8_t sda, uint8_t scl, uint8_t addr, int width, int height)
    : display(width, height, &Wire, -1),
      sdaPin(sda),
      sclPin(scl),
      i2cAddress(addr),
      screenWidth(width),
      screenHeight(height) {}

bool DisplayModule::init() {
    Wire.begin(sdaPin, sclPin);
    if (!display.begin(SSD1306_SWITCHCAPVCC, i2cAddress)) {
        return false;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    return true;
}

void DisplayModule::showDistance(float distance) {
    display.clearDisplay();// 清除显示屏
    display.setCursor(0, 0);// 设置光标位置
    display.print("Distance:");// 显示距离
    display.setCursor(0, 16);// 设置光标位置
    display.setTextSize(1);// 设置文本大小
    display.print(distance, 1);// 显示距离值，保留1位小数
    display.print(" cm");// 显示单位
    display.display();// 刷新显示屏
}

void DisplayModule::showSystemStatus(const String& status) {
    display.clearDisplay();// 清除显示屏
    display.setTextSize(1);// 设置文本大小
    display.setCursor(0, 0);// 设置光标位置
    display.print("System Status:");// 显示系统状态
    display.setCursor(0, 16);// 设置光标位置
    display.print(status);// 显示status（状态）信息
    display.display();// 刷新显示屏
}

void DisplayModule::showBootAnimation() {
    // 效果1：跳动的小球
    int radius = 5;
    int x = screenWidth / 2;
    int y = 0;
    int dy = 2;
    for (int i = 0; i < 100; i++) {
        display.clearDisplay();
        display.fillCircle(x, y, radius, SSD1306_WHITE);
        display.display();
        y += dy;
        if (y <= 0 || y >= screenHeight - radius) dy = -dy;
        delay(10);
    }

    // 效果2：进度计数器
    display.clearDisplay();
    display.setTextSize(1);
    for (int counter = 0; counter <= 100; counter++) {
        display.setCursor(0, 0);
        display.print("Progress: ");
        display.print(counter);
        display.println("%");

        // 绘制进度条
        display.drawRect(0, 15, screenWidth, 10, SSD1306_WHITE);
        display.fillRect(0, 15, map(counter, 0, 100, 0, screenWidth), 10, SSD1306_WHITE);

        // 在进度条下方添加"Initializing..."
        display.setCursor(0, 30);
        display.print("Initializing...");

        display.display();
        delay(15);
        display.clearDisplay();
    }
}

void DisplayModule::drawBattery(int x, int y, int percentage) {
    // 电池外框
    display.drawRect(x, y, 20, 10, SSD1306_WHITE);
    display.fillRect(x + 20, y + 2, 2, 6, SSD1306_WHITE);
    
    // 电池电量
    if (percentage > 0) {
        display.fillRect(x + 2, y + 2, (int)((16 * percentage) / 100), 6, SSD1306_WHITE);
    }
}

void DisplayModule::clearSection(int x, int y, int w, int h) {
    display.fillRect(x, y, w, h, SSD1306_BLACK);
}

void DisplayModule::drawDirectionArrow(int direction) {
    // 在屏幕中央绘制箭头 (coordinates for 128x64 display)
    const int centerX = 64;
    const int centerY = 40;
    
    switch(direction) {
        case FORWARD:
            display.fillTriangle(centerX, centerY-8, centerX-8, centerY+8, centerX+8, centerY+8, SSD1306_WHITE);// 向上箭头
            break;
        case BACKWARD:
            display.fillTriangle(centerX, centerY+8, centerX-8, centerY-8, centerX+8, centerY-8, SSD1306_WHITE);// 向下箭头
            break;
        case LEFT:
            display.fillTriangle(centerX-8, centerY, centerX+8, centerY-8, centerX+8, centerY+8, SSD1306_WHITE);// 向左箭头
            break;
        case RIGHT:
            display.fillTriangle(centerX+8, centerY, centerX-8, centerY-8, centerX-8, centerY+8, SSD1306_WHITE);// 向右箭头
            break;
    }
}

void DisplayModule::drawBleIcon(int x, int y, bool connected) {
    // 使用小三角形代替蓝牙图标
    if (connected || ((millis() / 500) % 2)) {  // 连接时常亮，断开时快速闪烁（200ms间隔）
        display.fillTriangle(
            x, y+4,      // 三角形顶点
            x+4, y,      // 右上角
            x+4, y+8,    // 右下角
            SSD1306_WHITE
        );
    }
}

void DisplayModule::updateDisplayData(float distance, int batteryLevel, const String& status, 
                                    int direction, bool bleConnected, const String& roomInfo) {
    display.clearDisplay();
    
    // 顶部状态栏
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(status);
    drawBleIcon(screenWidth-10, 0, bleConnected);
    display.drawLine(0, 9, screenWidth-1, 9, SSD1306_WHITE);
    
    // 左侧 - 距离
    display.setTextSize(2);
    display.setCursor(0, 16);
    if (distance >= 2 && distance <= 300) {
        display.print((int)distance);
        display.setTextSize(1);
        display.print("cm");
        
        // 添加房间号显示
        if (roomInfo.length() > 0) {
            display.setCursor(0, 35);  // 在距离显示下方
            display.print(roomInfo);
        }
    } else {
        display.setTextSize(1);
        display.print("Invalid");
    }
    
    // 右侧 - 电池
    drawBattery(screenWidth-22, 16, batteryLevel);
    display.setTextSize(1);
    display.setCursor(screenWidth-20, 28);
    display.print(String(batteryLevel) + "%");
    
    // 中央 - 方向箭头
    drawDirectionArrow(direction);
    
    // 底部信息
    display.drawLine(0, screenHeight-12, screenWidth-1, screenHeight-12, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, screenHeight-10);
    display.print("S3-Robot WangZhuo");
    
    display.display();
}