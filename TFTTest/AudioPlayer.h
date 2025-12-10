#pragma once
#include "DFRobotDFPlayerMini.h"
class UIManager;
class AudioPlayer {
public:
    AudioPlayer();   // 建構子
    // [修改] begin 接收 UIManager 指標
    void begin(UIManager* uiPtr);
    void update();
     void play();
    void pause();
    void next();
    void previous();
    void setVolume(int volume);
private:
    HardwareSerial FPSerial = HardwareSerial(1);
    DFRobotDFPlayerMini player;
    unsigned long lastAutoNextTime = 0;
    UIManager* ui; // [新增] 儲存 UI 物件的指標
    
    // [新增] 記錄目前歌曲索引 (假設有 5 首)
    int currentSongIndex = 1;
    const int totalSongs = 5;
};
