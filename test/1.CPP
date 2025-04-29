void loop() {
    // 处理WiFi状态
    handleWiFiStatus();

    // 网络连接正常时的主要控制逻辑
    if (WiFi.status() == WL_CONNECTED) {
        Blinker.run();
        
        // 根据模式执行相应逻辑
        if (isAutoMode) {
            handleAutoMode();
        }
        
        // 处理系统监控
        handleBatteryMonitor();
        handleUltrasonicData();
        handleNFCMonitor();
    }
}






    // 初始化Blinker前等待连接
    Serial.println("正在尝试连接 Wi-Fi");
    BLINKER_LOG("正在尝试连接 Wi-Fi");
    WiFi.begin(ssid, pswd); // 连接Wi-Fi

    // 等待WiFi连接完成
    uint32_t connectStart = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - connectStart < 15000) { // 15秒超时
        delay(500);
        Serial.print(".");
        BLINKER_LOG("连接中...");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
        BLINKER_LOG("连接成功！");
        // WiFi连接成功后再初始化Blinker
        Blinker.begin(auth, ssid, pswd);
        Blinker.attachData(dataRead);
    } else {
        Serial.println("\nConnection failed!");
        BLINKER_LOG("连接失败！");
    }