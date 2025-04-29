// 送餐服务处理函数
#include "ESP32Servo.h"
#include "DisplayModule.h"
class FoodService {
    private:
        Servo& foodCoverServo;
        DisplayModule& display;
        unsigned long serviceStartTime = 0;
        bool isServicing = false;
        bool coverOpened = false;
        uint8_t ledBlinkCount = 0;
        unsigned long lastBlinkTime = 0;
    
    public:
        FoodService(Servo& servo, DisplayModule& oled) 
            : foodCoverServo(servo), display(oled) {}
    
        void handle() {
            if (!isServicing) {
                startService();
                return;
            }
            
            unsigned long currentTime = millis();
            handleBlinkingAndCover(currentTime);
        }
    
    private:
        void startService() {
            serviceStartTime = millis();
            isServicing = true;
            coverOpened = true;
            foodCoverServo.write(90);
            display.showSystemStatus("Opening Cover");// 显示打开餐盖信息
        }
    
        void handleBlinkingAndCover(unsigned long currentTime) {

           // 30秒后开始LED闪烁 
            if (currentTime - serviceStartTime >= 30000 && coverOpened) {
                if (currentTime - lastBlinkTime >= 500) {
                    lastBlinkTime = currentTime;
                    if (ledBlinkCount < 10) {  // 闪烁10次后关闭餐盖    
                        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                        ledBlinkCount++;
                    } else {
                        closeService();
                    }
                }
            }
        }
    
        void closeService() {
            foodCoverServo.write(0);
            display.showSystemStatus("Closing Cover");// 显示关闭餐盖信息
            digitalWrite(LED_BUILTIN, LOW); // 关闭LED
            isServicing = false;// 服务结束
            coverOpened = false;// 餐盖关闭
            ledBlinkCount = 0;// 重置闪烁计数器
        }
    };