#pragma once
#include <TFT_eSPI.h>
#include <lvgl.h>

class UIManager {
public:
    void begin();
    void update();

    void resetUI();
    void updateProgress();
    void showQRCode(const char* text);
    void labelStatusUpdate(const char* text);

    void updateMusicInfo(int songIndex);
    // [新增] 音樂播放器相關函式
    void showMusicScreen();              // 切換到音樂畫面
    void updateMusicPlayState(bool play);// 更新播放/暫停按鈕圖示
private:
    static void flushCallback(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
   
    // [新增] 內部用：建立音樂介面
    void buildMusicUI();
    
    // [新增] 內部用：按鈕事件回呼 (static 才能被 LVGL 呼叫)
    static void event_handler_stub(lv_event_t * e);

    TFT_eSPI tft = TFT_eSPI();

    lv_obj_t *labelStatus;
    lv_obj_t *labelTitle;
    lv_obj_t *progressBar;
    lv_obj_t *qrContainer;

    // [新增] 音樂 UI 物件
    lv_obj_t *screenMusic;      // 音樂主畫面
    lv_obj_t *musicCover;       // 專輯封面容器
    lv_obj_t *labelSongTitle;   // 歌名
    lv_obj_t *labelArtist;      // 歌手
    lv_obj_t *barMusicProgress; // 進度條
    lv_obj_t *labelMusicTime;   // 時間顯示
    lv_obj_t *btnPlay;          // 播放/暫停按鈕
    lv_obj_t *screenMain; // 目前畫面
    lv_disp_draw_buf_t drawBuf;
    static const uint16_t screenWidth  = 320;
    static const uint16_t screenHeight = 240;
    lv_color_t buf[screenWidth * screenHeight / 10];
    const char* songTitles[6] = {
        "", // 0號留空，因為我們從 1 開始
        "Song Title 1",
        "Song Title 2",
        "Song Title 3",
        "Song Title 4",
        "Song Title 5"
    };

public:
    int progressStep = 0;  
    bool isMusicCreated = false; // 標記是否已經建立過音樂介面
};
