#define BLINKER_WIFI
//#define BLINKER_LAN  // 启用Blinker的LAN模式
#include <Blinker.h>
#include <Ultrasonic.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MotorController.h"
#include "DisplayModule.h"
#include "battery_monitor.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// 超声波配置
const uint8_t trigPin = 16; // 超声波触发引脚,GPIO16-trig
const uint8_t echoPin = 4;  // 超声波接收引脚,GPIO4-echo
Ultrasonic ultrasonic(trigPin, echoPin);

// OLED配置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_I2C_ADDR 0x3C
#define OLED_SDA 21 // OLED的SDA引脚，GPIO21-SDA
#define OLED_SCL 22 // OLED的SCL引脚，GPIO22-SCL

// Blinker配置
char auth[] = "bf090b26587a";   // 设备密钥
char ssid[] = "123";    // WiFi网络名称
char pswd[] = "1234567888"; // WiFi密码

// Blinker 组件
BlinkerNumber Number1("D");  // 定义Blinker APP超声波距离数据显示组件
BlinkerNumber Number2("L");  // 定义Blinker APP电池电量数据显示组件
BlinkerNumber Number3("V");    // 定义Blinker APP电池电压数据显示组件
BlinkerButton ButtonForward("A");  // 定义Blinker APP前进按钮
BlinkerButton ButtonBackward("B");    // 定义Blinker APP后退按钮
BlinkerButton ButtonLeft("L");   // 定义Blinker APP左转按钮
BlinkerButton ButtonRight("R"); // 定义Blinker APP右转按钮
BlinkerSlider SliderSpeed("S"); // 定义Blinker APP速度调节滑块

BatteryMonitor battery; // 声明电池监控对象

// 全局对象
MotorController motorController;
DisplayModule oledDisplay(OLED_SDA, OLED_SCL, OLED_I2C_ADDR, SCREEN_WIDTH, SCREEN_HEIGHT); // 声明OLED显示器对象

// 定时变量
unsigned long previousUltrasonicMillis = 0; // 超声波数据上报时间，非阻塞方式，通过millis()计时器
unsigned long previousBatteryMillis = 0;    // 电池数据上报时间，非阻塞方式，通过millis()计时器
unsigned long previousLEDMillis = 0;    //LED闪烁时间,非阻塞方式，通过millis()计时器
const long ultrasonicInterval = 1000; // 超声波数据上报间隔为1秒
const long batteryInterval = 6000;   // 电池数据上报间隔为6秒
bool ledState = LOW;    // LED状态

// 电机速度设定值
float setSpeed = 0;

void dataRead(const String &data) {
    BLINKER_LOG("Received data: ", data);

    if (data.indexOf("D") != -1 && data.indexOf("T") != -1 && data.indexOf("Y") != -1) {
        // 非阻塞处理指令
        // 避免使用耗时操作
        // 可以在这里添加异步处理逻辑
    }
}

// 回调函数
void forwardCallback(const String &state) { // 前进按钮回调函数
    if (state == "press") {
        motorController.moveForward();  // 调用前进函数
        BLINKER_LOG("Forward");
    } else if (state == "pressup") {
        motorController.stop();
    }
    if (state == "tap") {
        motorController.moveForward();
        BLINKER_LOG("Forward");
        delay(200);
        motorController.stop();
    }
}

void backwardCallback(const String &state) {    // 后退按钮回调函数
    if (state == "press") {
        motorController.moveBackward(); // 调用后退函数
        BLINKER_LOG("Backward");    
    } else if (state == "pressup") {
        motorController.stop();
    }
    if (state == "tap") {
        motorController.moveBackward();
        BLINKER_LOG("Backward");
        delay(200);
        motorController.stop();
    }
}

void leftCallback(const String &state) {    // 左转按钮回调函数
    if (state == "press") {
        motorController.turnLeft(); // 调用左转函数
        BLINKER_LOG("Left Turn");
    } else if (state == "pressup") {
        motorController.stop();
    }
    if (state == "tap") {
        motorController.turnLeft();
        BLINKER_LOG("Left Turn");
        delay(200);
        motorController.stop();
    }
}

void rightCallback(const String &state) {   // 右转按钮回调函数
    if (state == "press") {
        motorController.turnRight();    // 调用右转函数
        BLINKER_LOG("Right Turn");
    } else if (state == "pressup") {
        motorController.stop();
    }
    if (state == "tap") {
        motorController.turnRight();
        BLINKER_LOG("Right Turn");
        delay(200);
        motorController.stop();
    }    
}

void speedCallback(int32_t speed) {     // 速度调节滑块回调函数
    setSpeed = speed;
    motorController.setSpeed(setSpeed); // 调用设置速度函数
    motorController.stop();         
    BLINKER_LOG("Speed Setpoint: ", setSpeed);                     // 打印设置的速度
}


unsigned long lastReconnectAttempt = 0; // 上次重连尝试时间
uint32_t reconnectAttempts = 0;  // 记录重连尝试次数
uint32_t reconnectInterval = 500;  // 初始间隔500ms
const uint32_t maxInterval = 60000; // 最大间隔60秒

void smartReconnect() {
    if (WiFi.status() == WL_CONNECTED) return;

    unsigned long now = millis();
    
    // 带溢出保护的时间比较
    if ((now - lastReconnectAttempt) >= reconnectInterval) {
        Serial.printf("尝试第%d次重连，间隔%ums\n", 
                    reconnectAttempts, reconnectInterval);
        
        // 标准重连流程
        WiFi.disconnect();          // 清除旧连接
        WiFi.begin(ssid, pswd); // 重新初始化
        // 或者使用 WiFi.reconnect() 根据情况选择       
        // 更新重连参数
        lastReconnectAttempt = now;
        reconnectInterval = min(reconnectInterval * 2, maxInterval);
    }
}

// 网络监控任务
void networkMonitorTask(void *pvParameters) {
    while (1) {
        wl_status_t wifiStatus = WiFi.status(); // 获取当前WiFi连接状态

        if (wifiStatus != WL_CONNECTED) {
            // 断网时的 LED 闪烁（非阻塞）
            unsigned long currentMillis = millis();
            if (currentMillis - previousLEDMillis >= 500) { // 500ms 间隔
                previousLEDMillis = currentMillis;
                ledState = !ledState;
                digitalWrite(LED_BUILTIN, ledState);
                motorController.stop(); // 断网时停止电机
            }
            // 自动重连
            smartReconnect();
        } else {
            // 网络正常时保持 LED 熄灭
            digitalWrite(LED_BUILTIN, LOW);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 每100ms检查一次网络状态
    }
}

// 超声波数据上报任务
void ultrasonicTask(void *pvParameters) {
    while (1) {
        unsigned long currentMillis = millis();

        // 超声波数据上报处理
        if (currentMillis - previousUltrasonicMillis >= ultrasonicInterval) {
            previousUltrasonicMillis = currentMillis;

            // 获取传感器数据
            float distance = ultrasonic.getDistanceCM();  // 获取距离

            // 超声波数据上报到Blinker
            if (distance >= 2 && distance <= 400) {
                Number1.print(distance);
                oledDisplay.showDistance(distance);
            } else {
                oledDisplay.showSystemStatus("Invalid Distance");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 每100ms检查一次是否需要上报超声波数据
    }
}

// 电池数据上报任务
void batteryTask(void *pvParameters) {
    while (1) {
        unsigned long currentMillis = millis();

        // 电池数据上报处理
        if (currentMillis - previousBatteryMillis >= batteryInterval) {
            previousBatteryMillis = currentMillis;

            // 电池数据更新
            battery.update();

            // 获取电池数据
            float batteryVoltage = battery.getVoltage();  // 获取电池电压
            int batteryLevel = battery.getLevel();        // 获取电池电量

            // 电池数据上报blinker
            Number2.print(batteryLevel);
            Number3.print(batteryVoltage);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 每100ms检查一次是否需要上报电池数据
    }
}

// Blinker运行任务
void blinkerTask(void *pvParameters) {
    while (1) {
        Blinker.run();
        vTaskDelay(pdMS_TO_TICKS(10)); // 每10ms运行一次Blinker
    }
}

void setup() {
    Serial.begin(115200);
    BLINKER_DEBUG.stream(Serial);   // 启用Blinker调试输出

   // Blinker.setKeepAlive(30); // 设置心跳间隔为30秒

    // 电池监控初始化
    battery.begin();

    // 初始化OLED
    if (!oledDisplay.init()) {
        Serial.println("OLED initialization failed!");
        while (1);
    }
    oledDisplay.showSystemStatus("Initializing...");

    // 初始化电机控制器
    motorController.begin();
    motorController.stop();

    // 初始化LED 
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);


    // 强制设置为STA模式
    WiFi.mode(WIFI_STA);

    // 初始化Blinker前等待连接
    Serial.println("正在尝试连接 Wi-Fi");

    // 初始化Blinker
    Blinker.begin(auth, ssid, pswd);
    Blinker.attachData(dataRead);

    // 等待连接完成
    uint32_t connectStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectStart < 15000) { // 15秒超时
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\nConnection failed!");
    }

    // 绑定回调函数
    ButtonForward.attach(forwardCallback);
    ButtonBackward.attach(backwardCallback);
    ButtonLeft.attach(leftCallback);
    ButtonRight.attach(rightCallback);
    SliderSpeed.attach(speedCallback);

    Serial.println("系统初始化");

    // 创建任务
#if !USE_MULTCORE
    xTaskCreate(
        networkMonitorTask,   
        "networkMonitorTask", 
        4096,         
        NULL,         
        1,            
        NULL);        

    xTaskCreate(
        ultrasonicTask,   
        "ultrasonicTask", 
        4096,       
        NULL,       
        2,          
        NULL);      

    xTaskCreate(
        batteryTask,   
        "batteryTask", 
        4096,       
        NULL,       
        3,         
        NULL);      

    xTaskCreate(
        blinkerTask,   
        "blinkerTask", 
        4096,       
        NULL,       
        2,          
        NULL);      
#else
    //最后一个参数至关重要，决定这个任务创建在哪个核上.PRO_CPU 为 0, APP_CPU 为 1,或者 tskNO_AFFINITY 允许任务在两者上运行.
    xTaskCreatePinnedToCore(networkMonitorTask, "networkMonitorTask", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(ultrasonicTask, "ultrasonicTask", 4096, NULL, 3, NULL, 1);
    xTaskCreatePinnedToCore(batteryTask, "batteryTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(blinkerTask, "blinkerTask", 4096, NULL, 4, NULL, 0);
#endif

    vTaskDelete(NULL); // 删除主任务
}

void loop() {
    // 不会执行到这里，因为主任务已被删除
}