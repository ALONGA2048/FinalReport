#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

HardwareSerial FPSerial(1);  // UART2

DFRobotDFPlayerMini myDFPlayer;

void setup() {
  Serial.begin(115200);

  // 改用 UART2 → GPIO17 (RX) , GPIO18 (TX)
  FPSerial.begin(9600, SERIAL_8N1, 21, 17);
  
  Serial.println();
  Serial.println("DFPlayer Mini Demo");
  Serial.println("Initializing DFPlayer ...");

  if (!myDFPlayer.begin(FPSerial, true, true)) {
    Serial.println("DFPlayer init failed!");
    Serial.println("1.Check wiring");
    Serial.println("2.Check SD card");
    while (1) {}
  }

  Serial.println("DFPlayer Mini online!");

  myDFPlayer.volume(10);
  myDFPlayer.play(2);
}

void loop() {
  if (myDFPlayer.available()) {
    uint8_t type = myDFPlayer.readType();
    uint16_t value = myDFPlayer.read();

    switch (type) {

      // 播放完成事件
      case DFPlayerPlayFinished:
        Serial.print("▶ 播放完成：曲目 ");
        Serial.println(value);
        break;

      // 狀態事件（例如開機、設定）
      case DFPlayerFeedBack:
        Serial.print("ℹ️ 指令回應（Feedback）：");
        Serial.println(value);
        break;

      // 記憶卡事件（插入/拔除）
      case DFPlayerCardInserted:
        Serial.println("📥 SD 卡插入");
        break;

      case DFPlayerCardRemoved:
        Serial.println("📤 SD 卡拔除");
        break;

      // 錯誤事件
      case DFPlayerError:
        Serial.print("❌ 錯誤：");
        
        break;

      default:
        Serial.print("其它事件 Type=");
        Serial.print(type);
        Serial.print(" Value=");
        Serial.println(value);
        break;
    }
  }
}
