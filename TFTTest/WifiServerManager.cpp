#include "WifiServerManager.h"

WifiServerManager::WifiServerManager(UIManager* ui,AudioPlayer* audio) 
     : ui(ui), audio(audio)
{

}
   


void WifiServerManager::begin() {
    WiFi.config(local_IP, gateway, subnet);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected");
    server.begin();
}

void WifiServerManager::update() {
    WiFiClient client = server.available();
    if (!client || !client.available()) return;
    // 等待客戶端傳送資料 (這很重要，有時候連線建立了但資料還沒傳過來)
    // 最多等 500ms
    int timeout = 500; 
    while (!client.available() && timeout > 0) {
        delay(1);
        timeout--;
    }

    // 如果等了半天還是沒資料，可能只是個 Ping 或錯誤連線
    if (!client.available()) {
        client.stop(); // 斷開
        return;
    }
    String msg = client.readStringUntil('\n');
    msg.trim();

    Serial.println("Received: " + msg);

    if (msg.startsWith("new")) {
        ui->resetUI();
    }
    else if (msg.startsWith("http")) {
        ui->showQRCode(msg.c_str());
        ui->progressStep++;
    }else if (msg.startsWith("player")) {
        ui->showMusicScreen(); // [新增] 切換畫面到音樂播放器
        int cmd = msg.substring(6).toInt();  // player 1,2,3,4

        switch (cmd) {
            case 1:
                audio->play();
                ui->updateMusicPlayState(true); // [新增] 更新 UI 為 "暫停圖示" (表示正在播)
                break;

            case 2:
                audio->pause();
                ui->updateMusicPlayState(false); // [新增] 更新 UI 為 "播放圖示"
                break;

            case 3:
                audio->previous();
                ui->labelStatusUpdate("Music: Previous");
                break;

            case 4:
                audio->next();
                ui->labelStatusUpdate("Music: Next");
                break;

            default:
                Serial.println("Unknown player cmd");
                break;
        }
    }else if (msg.startsWith("volume")) {
        int vol = msg.substring(6).toInt(); 
        audio->setVolume(vol);
        
        // 選擇性：如果你想在螢幕上顯示音量變化，可以使用這行
        // ui->labelStatusUpdate("Vol: " + String(vol)); 
    }else if (ui->progressStep < 4) {
        ui->updateProgress();
    }

    client.println("OK");
}
