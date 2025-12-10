#pragma once
#include <WiFi.h>
#include "UIManager.h"
#include "AudioPlayer.h"

class WifiServerManager {
public:
    WifiServerManager(UIManager* ui, AudioPlayer* audio);

    void begin();
    void update();

private:
    UIManager* ui;
    AudioPlayer* audio;

    const char* ssid     = "密碼是password";
    const char* password = "17802356";
    const int serverPort = 8080;

    IPAddress local_IP  {10, 14, 159, 101};
    IPAddress gateway   {10, 14, 159, 175};
    IPAddress subnet    {255, 255, 255, 0};

    WiFiServer server = WiFiServer(serverPort);
};
