#define BLINKER_PRINT Serial
#define BLINKER_BLE // 启用Blinker BLE
#include <Blinker.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MotorController.h"
#include "DisplayModule.h"
#include "battery_monitor.h"
#include "tracking_sensor.h"
#include "NFCReader.h"
#include "Ultrasonic.h"
#include "ESP32Servo.h"
#include "FoodService.h"

// Blinker 组件
BlinkerNumber Number1("D");  // 定义Blinker APP障碍距离数据显示组件
BlinkerNumber Number2("L");  // 定义Blinker APP电池电量数据显示组件
BlinkerNumber Number3("V");  // 定义Blinker APP电池电压数据显示组件
BlinkerNumber Number4("Y");  // 定义Blinker APP移动速度数据显示组件
BlinkerButton ButtonForward("A");  // 定义Blinker APP前进按钮
BlinkerButton ButtonBackward("B");    // 定义Blinker APP后退按钮
BlinkerButton ButtonLeft("L");   // 定义Blinker APP左转按钮
BlinkerButton ButtonRight("R"); // 定义Blinker APP右转按钮
BlinkerSlider SliderSpeed("S"); // 定义Blinker APP速度调节滑块
BlinkerButton Button1("C"); // 定义Blinker APP餐盖开关按钮

BatteryMonitor battery; // 声明电池监控对象
TrackingSensor tracker; // 声明循迹传感器对象
NFCReader nfc;// 声明NFC读卡器对象
MotorController motorController;// 声明电机控制对象
DisplayModule oledDisplay(OLED_SDA, OLED_SCL, OLED_I2C_ADDR, SCREEN_WIDTH, SCREEN_HEIGHT); // 声明OLED显示器对象

// 超声波传感器配置
const int TRIG_PIN = 15; // 超声波触发引脚,GPIO15-trig
const int ECHO_PIN = 16;  // 超声波接收引脚,GPIO16-echo
Ultrasonic ultrasonic(TRIG_PIN, ECHO_PIN);  // 声明超声波对象

// 定时变量
unsigned long previousUltrasonicMillis = 0; // 超声波数据上报时间，非阻塞方式，通过millis()计时器
unsigned long previousBatteryMillis = 0;    // 电池数据上报时间，非阻塞方式，通过millis()计时器
unsigned long previousLEDMillis = 0;    //LED闪烁时间,非阻塞方式，通过millis()计时器
const long ultrasonicInterval = 800; // 超声波数据上报间隔为0.8秒
const long batteryInterval = 3000;   // 电池数据上报间隔为3秒
bool ledState = LOW;    // LED状态

// 电机速度设定值
float setSpeed = 70;

// 速度显示相关变量
unsigned long previousSpeedMillis = 0;    // 速度更新计时器
const long speedUpdateInterval = 800;      // 速度更新间隔(800ms)
float currentDisplaySpeed = 0;            // 当前显示速度
unsigned long speedChangeStartTime = 0;    // 速度变化开始时间

// 在全局变量区域添加 NFC 显示控制变量
unsigned long nfcDisplayStartTime = 0;  // NFC信息显示开始时间
const unsigned long NFC_DISPLAY_DURATION = 2500;  // NFC信息显示持续时间（2.5秒）
String lastNfcInfo = "";  // 上次显示的NFC信息

// 在handleUltrasonicData函数之前添加函数声明
void handleDistanceBeep(float distance);

// 速度更新函数
void updateSpeed() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousSpeedMillis >= speedUpdateInterval) {
        previousSpeedMillis = currentMillis;
        
        // 更新实际速度
        motorController.updateEncoders();
        float actualSpeed = motorController.getAverageSpeed() * 1000000; // 转换为cm/s (从m/s转换)
        
        // 更新显示
        currentDisplaySpeed = actualSpeed;
        Number4.print(currentDisplaySpeed);
    }
}
// 加速函数

// 添加检查函数
bool isDisplayingNFC() {
    return (nfcDisplayStartTime > 0 && (millis() - nfcDisplayStartTime < NFC_DISPLAY_DURATION));// 检查是否正在显示NFC信息
}

void handleBatteryMonitor() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousBatteryMillis >= batteryInterval) {
        previousBatteryMillis = currentMillis;
        battery.update();
        
        float batteryVoltage = battery.getVoltage();
        int batteryLevel = battery.getLevel();
        
        // 电池状态上报
        Number2.print(batteryLevel);
        Number3.print(batteryVoltage);
        
        // 只有在不显示NFC信息时才更新显示
        if (!isDisplayingNFC()) {
            float distance = ultrasonic.getDistanceCM();
            String status = (batteryLevel < 30) ? "Low Battery!" : "System Normal";
            oledDisplay.updateDisplayData(distance, batteryLevel, status, 
                                    DisplayModule::NONE, Blinker.connected(), "");
        }
    }
}

void handleUltrasonicData() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousUltrasonicMillis >= ultrasonicInterval) {
        previousUltrasonicMillis = currentMillis;
        float distance = ultrasonic.getDistanceCM();
        
        if (distance >= 2 && distance <= 400) {
            Number1.print(distance);
            // 只有在不显示NFC信息时才更新显示
            if (!isDisplayingNFC()) {
                int batteryLevel = battery.getLevel();
                String status = "System Normal";
                oledDisplay.updateDisplayData(distance, batteryLevel, status, 
                                    DisplayModule::NONE, Blinker.connected(), "");
            }
            // 添加距离提示音处理
            handleDistanceBeep(distance);
        }
    }
}

void dataRead(const String &data) {
    BLINKER_LOG("Received data: ", data);

    /*if (data.indexOf("D") != -1 && data.indexOf("T") != -1 && data.indexOf("Y") != -1) {
        // 这里检查数据中是否包含"D"、"T"和"Y"三个字符
        // 如果都包含，则执行相应操作（目前为空）   
        // 非阻塞处理指令
        // 避免使用耗时操作
        // 可以在这里添加异步处理逻辑
    }*/
}

// 回调函数
void forwardCallback(const String &state) {
    if (state == "press") {
        motorController.setSpeed(setSpeed);
        motorController.moveForward();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "System Normal", DisplayModule::FORWARD, Blinker.connected());
        BLINKER_LOG("Forward");
    } else if (state == "pressup") {
        motorController.stop();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "Manual Mode", DisplayModule::NONE, Blinker.connected());
    }
}


void backwardCallback(const String &state) {
    if (state == "press") {
        motorController.setSpeed(setSpeed);
        motorController.moveBackward();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "System Normal", DisplayModule::BACKWARD, Blinker.connected());
        BLINKER_LOG("Backward");
    } else if (state == "pressup") {
        motorController.stop();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "Manual Mode", DisplayModule::NONE, Blinker.connected());
    }
}


void leftCallback(const String &state) {
    if (state == "press") {
        motorController.setSpeed(setSpeed);
        motorController.turnLeft();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "System Normal", DisplayModule::LEFT, Blinker.connected());
        BLINKER_LOG("Left Turn");
    } else if (state == "pressup") {
        motorController.stop();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "Manual Mode", DisplayModule::NONE, Blinker.connected());
    }
}

void rightCallback(const String &state) {
    if (state == "press") {
        motorController.setSpeed(setSpeed);
        motorController.turnRight();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "System Normal", DisplayModule::RIGHT, Blinker.connected());
        BLINKER_LOG("Right Turn");
    } else if (state == "pressup") {
        motorController.stop();
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "Manual Mode", DisplayModule::NONE, Blinker.connected());
    }
}



void speedCallback(int32_t speed) {     // 速度调节滑块回调函数
    motorController.stop(); // 设定速度停止电机
    oledDisplay.showSystemStatus("Speed Setpoint: " + String(speed)); // 显示设定速度
    BLINKER_LOG("Speed Setpoint: ", speed); // 打印设定速度
    setSpeed = speed;
    BLINKER_LOG("Speed Setpoint: ", setSpeed); // 打印设置的速度
}

// 蜂鸣器控制相关变量
const int BUZZER_PIN = 2;  // 无源蜂鸣器引脚
const int BEEP_DURATION = 150;  // 每次蜂鸣持续时间调整至150ms
const int BEEP_INTERVAL = 300;  // 蜂鸣间隔时间调整至300ms
const int BEEP_FREQ = 4000;     // 蜂鸣频率调整至4000Hz，提高音量
unsigned long lastBeepMillis = 0;    // 上次蜂鸣器状态改变时间
int remainingBeeps = 0;              // 剩余蜂鸣次数
bool beepState = false;              // 当前蜂鸣器状态

// 在蜂鸣器控制相关变量部分修改
const int DISTANCE_BEEP_INTERVAL_1 = 150;  // 0-10cm的间隔改为100ms
const int DISTANCE_BEEP_INTERVAL_2 = 300;  // 10-20cm的间隔改为200ms
const int DISTANCE_BEEP_INTERVAL_3 = 500;  // 20-30cm的间隔保持不变
unsigned long lastDistanceBeepMillis = 0;   // 距离提示音计时器

// 添加距离提示音函数
void handleDistanceBeep(float distance) {
    unsigned long currentMillis = millis();
    
// 如果正在执行其他蜂鸣序列，不进行距离提示音
    if (remainingBeeps > 0) {
        return;
    }

    if (distance <= 10) {
        // 0-10cm，快而急促的提示音
        if (currentMillis - lastDistanceBeepMillis >= DISTANCE_BEEP_INTERVAL_1) {
            tone(BUZZER_PIN, BEEP_FREQ, 500);  // 持续时间改为100ms
            lastDistanceBeepMillis = currentMillis;
        }
    } else if (distance <= 20) {
        // 10-20cm，较快的提示音
        if (currentMillis - lastDistanceBeepMillis >= DISTANCE_BEEP_INTERVAL_2) {
            tone(BUZZER_PIN, BEEP_FREQ, 350);  // 持续时间改为200ms
            lastDistanceBeepMillis = currentMillis;
        }
    } else if (distance <= 30) {
        // 20-30cm，缓慢的提示音
        if (currentMillis - lastDistanceBeepMillis >= DISTANCE_BEEP_INTERVAL_3) {
            tone(BUZZER_PIN, BEEP_FREQ, 200);//  // 持续时间改为300ms
            lastDistanceBeepMillis = currentMillis;
        }
    } else {
        // 大于30cm，不发出提示音
        noTone(BUZZER_PIN);
    }
}

// 添加在蜂鸣器控制相关变量后，handleDistanceBeep函数前
// 开始蜂鸣器序列
void startBeepSequence(int times) {
    remainingBeeps = times * 2; // 每次蜂鸣需要开和关两个状态 
    lastBeepMillis = millis();
    beepState = true;
    tone(BUZZER_PIN, BEEP_FREQ);  // 启动第一次蜂鸣
}

// 更新蜂鸣器状态
void updateBuzzer() {
    if (remainingBeeps <= 0) {
        noTone(BUZZER_PIN);  // 确保蜂鸣器停止
        return;
    }
    
    unsigned long currentMillis = millis();
    if (beepState && currentMillis - lastBeepMillis >= BEEP_DURATION) {
        noTone(BUZZER_PIN);
        beepState = false;
        lastBeepMillis = currentMillis;
        remainingBeeps--;
        if (remainingBeeps <= 0) {
            noTone(BUZZER_PIN);  // 蜂鸣序列结束时确保停止
        }
    }
    else if (!beepState && currentMillis - lastBeepMillis >= BEEP_INTERVAL - BEEP_DURATION) {
        if (remainingBeeps > 0) {
            tone(BUZZER_PIN, BEEP_FREQ);
            beepState = true;
            lastBeepMillis = currentMillis;
            remainingBeeps--;
        }
    }
}

void button1Callback(const String &state) {
    BLINKER_LOG("收到按钮状态: ", state);
    Serial.printf("收到按钮控制命令: %s\n", state.c_str());
    
    if (state == "on") {
        Serial.println("处理开启命令");   
        Button1.print("on");
        Button1.text("餐盖已开启");
        startBeepSequence(3);  // 蜂鸣提示
        
        BLINKER_LOG("餐盖开启完成");
    }
    else if (state == "off") {
        Serial.println("处理关闭命令");
        Button1.print("off");
        Button1.text("餐盖已关闭");
        BLINKER_LOG("餐盖关闭完成");
    }
}

bool lastBleState = false;  // 保存上一次的蓝牙连接状态

// 添加蓝牙连接状态处理函数
void handleBleConnection() {
    bool currentBleState = Blinker.connected();
    
    if (lastBleState && !currentBleState) {
        motorController.stop();  // 停止电机
        if (!isDisplayingNFC()) {
            int batteryLevel = battery.getLevel();
            float distance = ultrasonic.getDistanceCM();
            oledDisplay.updateDisplayData(distance, batteryLevel, "BLE Disconnected", DisplayModule::NONE, false);
        }
        BLINKER_LOG("蓝牙已断开连接");
    }
    else if (!lastBleState && currentBleState) {
        BLINKER_LOG("蓝牙已连接");
        if (!isDisplayingNFC()) {
            int batteryLevel = battery.getLevel();
            float distance = ultrasonic.getDistanceCM();
            oledDisplay.updateDisplayData(distance, batteryLevel, "BLE Connected", DisplayModule::NONE, true);
        }
    }
    
    lastBleState = currentBleState;
}

// 送餐服务函数
void FoodService() {
    //暂时不实现送餐服务的具体逻辑
    // 这里可以添加送餐服务的具体实现代码   
}

// NFC处理函数
void handleNFCMonitor() {
    static unsigned long lastCheck = 0;
    unsigned long currentMillis = millis();

    if (currentMillis - lastCheck < 100) {
        return;
    }
    lastCheck = currentMillis;

    if (nfcDisplayStartTime > 0 && currentMillis - nfcDisplayStartTime >= NFC_DISPLAY_DURATION) {
        nfcDisplayStartTime = 0;
        lastNfcInfo = "";
        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, "System Normal", DisplayModule::NONE, Blinker.connected());
        return;
    }

    if (nfc.readCard()) {
        String uid = nfc.getUID();
        String cardType = nfc.getCardType();
        String roomInfo = nfc.getMappedRoom();
        
        Serial.println("NFC Card Detected!");
        BLINKER_LOG("NFC Card Detected!");
        Serial.println("Card Type: " + cardType);
        BLINKER_LOG("Card Type: ", cardType);
        Serial.println("UID: " + uid);
        BLINKER_LOG("UID: ", uid);
        Serial.println("Room: " + roomInfo);
        BLINKER_LOG("Room: ", roomInfo);

        lastNfcInfo = roomInfo;
        nfcDisplayStartTime = currentMillis;

        int batteryLevel = battery.getLevel();
        float distance = ultrasonic.getDistanceCM();
        oledDisplay.updateDisplayData(distance, batteryLevel, lastNfcInfo, 
                                    DisplayModule::NONE, Blinker.connected(), roomInfo);
    }
}

void setup() {
    Serial.begin(115200);

    // 初始化 SPI 引脚
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
    
    #if defined(BLINKER_PRINT)
        BLINKER_DEBUG.stream(BLINKER_PRINT);
    #endif

    // 初始化电机控制器
    motorController.begin();

    BLINKER_DEBUG.stream(Serial);

    // 电池监控初始化
    battery.begin();

    // 初始化 NFC 读卡器
    nfc.begin();
    Serial.println("NFC Reader initialized");

    // 循迹传感器初始化
    tracker.init();
    delay(1000);

    // 初始化OLED显示屏
    if (!oledDisplay.init()) {
        Serial.println("OLED initialization failed!");
        while (1);
    }
    oledDisplay.showBootAnimation(); //开机加载动画


    // 初始化blinker
    Blinker.begin();
    Button1.print("off");  // 设置按钮初始状态为off
    Button1.text("餐盖开启");  // 设置按钮初始文本
    
    // 绑定回调函数
    ButtonForward.attach(forwardCallback);
    ButtonBackward.attach(backwardCallback);
    ButtonLeft.attach(leftCallback);
    ButtonRight.attach(rightCallback);
    SliderSpeed.attach(speedCallback);
    Button1.attach(button1Callback);

    pinMode(BUZZER_PIN, OUTPUT);  // 设置蜂鸣器引脚为输出模式

    BLINKER_LOG("系统初始化完成！");
    oledDisplay.showSystemStatus("Initialization OK!");
    delay(1500);
}

void loop() {
    Blinker.run();// 运行Blinker
    handleBleConnection();// 处理蓝牙连接状态
    handleBatteryMonitor();// 处理电池监控数据
    handleUltrasonicData();// 处理超声波数据
    updateSpeed();// 更新速度显示
    handleNFCMonitor();// NFC处理函数
    updateBuzzer();// 更新蜂鸣器状态
    FoodService(); // 送餐服务函数,自动化送餐服务
}

