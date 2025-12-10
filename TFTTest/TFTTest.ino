#include "UIManager.h"
#include "WifiServerManager.h"
#include "AudioPlayer.h"
#include <SPI.h>
#include <SD.h>

#define SD_CS 18  
UIManager ui;
AudioPlayer audio;
WifiServerManager wifi(&ui,&audio);


void setup() {
    Serial.begin(115200);
    delay(100);
    SPI.begin(48, 47, 38, 18); // SCK, MISO, MOSI, CS (順序：SCK,MISO,MOSI,CS)
 
    // 初始化 SD 卡
    if(!SD.begin(SD_CS, SPI)){
        Serial.println("SD card init failed");
        return;
    }
    Serial.println("SD card ready");
    // 4. 開啟根目錄 ("/") 並開始列出檔案
    File root = SD.open("/");
    printDirectory(root, 0);
    ui.begin();
    wifi.begin();
    audio.begin(&ui);
}

void loop() {
    ui.update();
    wifi.update();
    audio.update();

    delay(10);
}
// 這是用來列出檔案的函式 (遞迴邏輯)
void printDirectory(File dir, int numTabs) {
  while (true) {
    // 讀取下一個檔案/資料夾
    File entry = dir.openNextFile();
    
    // 如果沒有下一個檔案了，就跳出迴圈
    if (!entry) {
      break;
    }

    // 根據層級印出 tab (縮排)，讓顯示比較有階層感
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }

    // 印出檔案名稱
    Serial.print(entry.name());

    // 判斷是資料夾還是檔案
    if (entry.isDirectory()) {
      Serial.println("/"); // 如果是資料夾，加個斜線
      // 呼叫自己，進去這個資料夾繼續列印 (遞迴)
      printDirectory(entry, numTabs + 1);
    } else {
      // 如果是檔案，印出檔案大小
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      Serial.println(" bytes");
    }
    
    // 關閉目前的檔案項目，準備讀下一個
    entry.close();
  }
}

