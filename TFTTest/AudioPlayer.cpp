#include "AudioPlayer.h"
#include "UIManager.h" // [關鍵] 在 cpp 這裡引入 Header
// 修改 begin
void AudioPlayer::begin(UIManager* uiPtr) {
    this->ui = uiPtr; // 儲存指標

    FPSerial.begin(9600, SERIAL_8N1, 21, 17);
    if (!player.begin(FPSerial, true, true)) {
        Serial.println("DFPlayer init failed!");
        while (1) {}
    }
    player.volume(15);
}
AudioPlayer::AudioPlayer() {
    // 如果沒有特別要做事，留空即可
}
void AudioPlayer::update() {
    if (player.available()) {
        uint8_t type = player.readType();
        int value = player.read(); // 讀取具體數值 (例如播完的是第幾首)，即使不用也要讀出來以清空緩衝區

        if (type == DFPlayerPlayFinished) {
            // --- 新增防護機制 ---
            // 檢查：距離上次自動切歌是不是已經過了 3 秒？
            if (millis() - lastAutoNextTime > 3000) { 
                
                Serial.print("Song ");
                Serial.print(value);
                Serial.println(" finished. Playing next...");
                
                next(); 
                
                // 更新計時器，鎖住接下來的 3 秒
                lastAutoNextTime = millis(); 
            } else {
                Serial.println("Ignored rapid finish signal.");
            }
        }
        
        // 你也可以在這裡處理其他錯誤訊息 (例如 SD 卡拔出等)，參考範例中的 printDetail
    }
}

void AudioPlayer::play() {
    player.start();       // 或 player.play(currentIndex)
}

void AudioPlayer::pause() {
    player.pause();
}

void AudioPlayer::next() {
    
    // 1. 硬體切換
    player.next(); 
    
    // 2. 軟體計數更新
    currentSongIndex++;
    if (currentSongIndex > totalSongs) {
        currentSongIndex = 1; // 超過最後一首回到第一首
    }
    
    // 3. 通知 UI 更新
    if (ui) {
        ui->updateMusicInfo(currentSongIndex);
    }

    
    
}
void AudioPlayer::setVolume(int volume) {
    // 限制音量範圍在 0 到 30 之間，避免錯誤數值
    if (volume > 30) volume = 30;
    if (volume < 0) volume = 0;
    
    player.volume(volume);
}

void AudioPlayer::previous() {
    player.previous();

    // 2. 軟體計數更新
    currentSongIndex--;
    if (currentSongIndex < 1) {
        currentSongIndex = totalSongs; // 第一首往前變成最後一首
    }

    // 3. 通知 UI 更新
    if (ui) {
        ui->updateMusicInfo(currentSongIndex);
    }
}
