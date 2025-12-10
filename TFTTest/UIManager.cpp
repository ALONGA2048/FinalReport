#include "UIManager.h"
#include <SD.h> // [新增] 需要引入 SD 函式庫
#include <SPI.h>
// --- LVGL File System Interface for Arduino SD ---
// 這些函式是用來讓 LVGL 看得懂 Arduino 的 File 物件

// 修改 UIManager.cpp 中的 sd_fs_open 函式
static void* sd_fs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode) {
    const char * fmode = (mode == LV_FS_MODE_WR) ? FILE_WRITE : FILE_READ;
    
    // [DEBUG] 印出 LVGL 嘗試讀取的路徑，幫你確認檔名是否正確
    Serial.print("[LVGL] Request open: "); 
    Serial.println(path);

    // [關鍵修正] 確保路徑開頭有 "/" (根目錄)
    String fullPath = String(path);
    if (!fullPath.startsWith("/")) {
        fullPath = "/" + fullPath;
    }
    
    // 檢查檔案是否存在
    if (!SD.exists(fullPath)) {
        Serial.print("[LVGL] ❌ File not found on SD: "); 
        Serial.println(fullPath);
        return NULL;
    }

    // 嘗試開啟
    File* file = new File(SD.open(fullPath, fmode));
    
    if (!file || !(*file)) {
        Serial.println("[LVGL] ❌ Open failed (Hardware error?)");
        delete file;
        return NULL;
    }
    
    Serial.println("[LVGL] ✅ File opened successfully!");
    return file;
}

static lv_fs_res_t sd_fs_close(lv_fs_drv_t * drv, void * file_p) {
    File* file = (File*)file_p;
    file->close();
    delete file; // 釋放記憶體
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_fs_read(lv_fs_drv_t * drv, void * file_p, void * buf, uint32_t btr, uint32_t * br) {
    File* file = (File*)file_p;
    *br = file->read((uint8_t*)buf, btr); // 讀取資料
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_fs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence) {
    File* file = (File*)file_p;
    // 簡單實作：通常 LVGL 傳入的是絕對位置，或配合 whence 計算
    if (whence == LV_FS_SEEK_SET) file->seek(pos);
    else if (whence == LV_FS_SEEK_CUR) file->seek(file->position() + pos);
    else if (whence == LV_FS_SEEK_END) file->seek(file->size() + pos);
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_fs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p) {
    File* file = (File*)file_p;
    *pos_p = file->position();
    return LV_FS_RES_OK;
}
// ------------------------------------------------

// 定義事件回呼 (橋接器)
void UIManager::event_handler_stub(lv_event_t * e) {
    // 這裡可以處理螢幕上的觸控事件
    // 目前我們先留空，或做簡單的 Log
    lv_obj_t * obj = lv_event_get_target(e);
    LV_LOG_USER("Button clicked: %p", obj);
}

void UIManager::flushCallback(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    UIManager* ui = (UIManager*)disp->user_data;

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    ui->tft.startWrite();
    ui->tft.setAddrWindow(area->x1, area->y1, w, h);
    ui->tft.pushColors((uint16_t*)color_p, w * h, true);

    ui->tft.endWrite();

    lv_disp_flush_ready(disp);
}

void UIManager::begin() {
    tft.begin();
    tft.setRotation(1);
    
    lv_init();
    // --- [新增] 註冊 SD 卡驅動 ---
    static lv_fs_drv_t drv;                   // 建立驅動物件 (必須是 static)
    lv_fs_drv_init(&drv);                     // 初始化
    drv.letter = 'S';                         // 設定磁碟代號為 'S' (SD Card)
    drv.open_cb = sd_fs_open;                 // 掛載 open 函式
    drv.close_cb = sd_fs_close;               // 掛載 close 函式
    drv.read_cb = sd_fs_read;                 // 掛載 read 函式
    drv.seek_cb = sd_fs_seek;                 // 掛載 seek 函式
    drv.tell_cb = sd_fs_tell;                 // 掛載 tell 函式
    lv_fs_drv_register(&drv);                 // 註冊給 LVGL
    // ---------------------------

    lv_disp_draw_buf_init(&drawBuf, buf, NULL, screenWidth * screenHeight / 10);
   
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = flushCallback;
    disp_drv.draw_buf = &drawBuf;
    disp_drv.user_data = this;
    lv_disp_drv_register(&disp_drv);

    // Build UI
    screenMain = lv_scr_act();
    lv_obj_set_style_bg_color(screenMain, lv_color_hex(0x202020), LV_PART_MAIN);

    labelTitle = lv_label_create(screenMain);
    lv_label_set_text(labelTitle, "Listing...");
    
    lv_obj_align(labelTitle, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_color(labelTitle, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelTitle, &lv_font_montserrat_20, 0);
    
    progressBar = lv_bar_create(screenMain);
    lv_obj_set_size(progressBar, 260, 20);
    lv_obj_align(progressBar, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_range(progressBar, 0, 100);

    labelStatus = lv_label_create(screenMain);
    lv_label_set_text(labelStatus, "ip: 10, 14, 159, 101");
    lv_obj_align(labelStatus, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_text_color(labelStatus, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_20, 0);
    
    qrContainer = lv_obj_create(screenMain);
    lv_obj_set_size(qrContainer, screenWidth, screenHeight);
    lv_obj_add_flag(qrContainer, LV_OBJ_FLAG_HIDDEN);
    // 預先建立音樂介面 (但不顯示)
    buildMusicUI();
    lv_timer_handler();
}

// [重點] 實作音樂介面建構
void UIManager::buildMusicUI() {
    if(isMusicCreated) return;

    screenMusic = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screenMusic, lv_color_hex(0x282c34), LV_PART_MAIN);

    // 1. 專輯封面 (灰色方塊作為容器)
    musicCover = lv_obj_create(screenMusic);
    lv_obj_set_size(musicCover, 140, 140); 
    lv_obj_align(musicCover, LV_ALIGN_LEFT_MID, 20, 0); 
    lv_obj_set_style_bg_color(musicCover, lv_color_hex(0x535964), LV_PART_MAIN);
    // [建議] 移除容器的捲動屬性，避免圖片略大時出現捲軸
    lv_obj_clear_flag(musicCover, LV_OBJ_FLAG_SCROLLABLE); 

    // --- [新增] 顯示圖片程式碼 ---
    lv_obj_t * imgArt = lv_img_create(musicCover); // 建立圖片物件，父物件為 musicCover
    
    // 設定圖片來源
    // "S:" 對應你在 begin() 設定的 drv.letter
    // "cover.bin" 請改成你 SD 卡內實際的 .bin 檔名
    lv_img_set_src(imgArt, "S:2.bin"); 
    
    // 將圖片在容器內置中
    lv_obj_center(imgArt);

    // 2. 歌名
    labelSongTitle = lv_label_create(screenMusic);
    lv_label_set_text(labelSongTitle, songTitles[1]);
    lv_obj_set_style_text_font(labelSongTitle, &lv_font_montserrat_20, 0); 
    lv_obj_set_style_text_color(labelSongTitle, lv_color_white(), 0);
    lv_obj_align(labelSongTitle, LV_ALIGN_TOP_RIGHT, -20, 30);

    // 3. 歌手
    labelArtist = lv_label_create(screenMusic);
    lv_label_set_text(labelArtist, "Artist Name");
    lv_obj_set_style_text_font(labelArtist, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(labelArtist, lv_color_hex(0xbbbbbb), 0);
    lv_obj_align(labelArtist, LV_ALIGN_TOP_RIGHT, -20, 60);

    // 4. 控制按鈕 (上一首 / 播放 / 下一首)
    // 播放按鈕
    btnPlay = lv_btn_create(screenMusic);
    lv_obj_set_size(btnPlay, 50, 50);
    lv_obj_align(btnPlay, LV_ALIGN_BOTTOM_RIGHT, -80, -30);
    lv_obj_add_event_cb(btnPlay, event_handler_stub, LV_EVENT_CLICKED, NULL); // 註冊點擊事件
    
    lv_obj_t * lblPlay = lv_label_create(btnPlay);
    lv_label_set_text(lblPlay, LV_SYMBOL_PLAY);
    lv_obj_center(lblPlay);

    // 上一首
    lv_obj_t * btnPrev = lv_btn_create(screenMusic);
    lv_obj_set_size(btnPrev, 40, 40);
    lv_obj_align_to(btnPrev, btnPlay, LV_ALIGN_OUT_LEFT_MID, -20, 0);
    
    lv_obj_t * lblPrev = lv_label_create(btnPrev);
    lv_label_set_text(lblPrev, LV_SYMBOL_PREV);
    lv_obj_center(lblPrev);

    // 下一首
    lv_obj_t * btnNext = lv_btn_create(screenMusic);
    lv_obj_set_size(btnNext, 40, 40);
    lv_obj_align_to(btnNext, btnPlay, LV_ALIGN_OUT_RIGHT_MID, 20, 0);

    lv_obj_t * lblNext = lv_label_create(btnNext);
    lv_label_set_text(lblNext, LV_SYMBOL_NEXT);
    lv_obj_center(lblNext);

    // 5. 進度條
    barMusicProgress = lv_bar_create(screenMusic);
    lv_obj_set_size(barMusicProgress, 140, 10);
    lv_obj_align(barMusicProgress, LV_ALIGN_BOTTOM_LEFT, 20, -20);
    lv_bar_set_value(barMusicProgress, 30, LV_ANIM_OFF);

    isMusicCreated = true;
}

// 切換到音樂畫面
void UIManager::showMusicScreen() {
    if(!isMusicCreated) buildMusicUI();
    lv_scr_load(screenMusic);
}

// 更新播放狀態 (圖示變更)
void UIManager::updateMusicPlayState(bool play) {
    if (!btnPlay) return;
    lv_obj_t * label = lv_obj_get_child(btnPlay, 0);
    if (play) {
        lv_label_set_text(label, LV_SYMBOL_PAUSE);
    } else {
        lv_label_set_text(label, LV_SYMBOL_PLAY);
    }
}

void UIManager::update() {
    lv_timer_handler();
}

void UIManager::resetUI() {
    if (lv_scr_act() != screenMain) {
        lv_scr_load(screenMain);
    }
    progressStep = 0;
    lv_label_set_text(labelTitle, "downloading...");
    lv_bar_set_value(progressBar, 0, LV_ANIM_OFF);

    lv_obj_clear_flag(labelTitle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(progressBar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(qrContainer, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clean(qrContainer);
}

void UIManager::updateProgress() {
    progressStep++;
    if (progressStep > 4) return;

    int percent = progressStep * 25;
    char text[32];
    sprintf(text, "processing: %d%%", percent);

    lv_label_set_text(labelTitle, text);
    lv_bar_set_value(progressBar, percent, LV_ANIM_ON);
}

void UIManager::updateMusicInfo(int songIndex) {
    if (!isMusicCreated) return; // 如果音樂介面還沒建立，就不執行

    // 1. 更新圖片來源
    // 組合路徑字串，例如 "S:1.bin"
    // 注意：LVGL 圖片來源字串必須是靜態的或持續存在的，但 lv_img_set_src 會立刻複製字串，所以這裡用暫存 buffer 沒問題
    char imgPath[16];
    sprintf(imgPath, "S:%d.bin", songIndex);
    
    Serial.print("Updating Image to: ");
    Serial.println(imgPath);

    // 取得 musicCover 裡面的圖片物件 (它是 musicCover 的第一個子物件)
    lv_obj_t * imgArt = lv_obj_get_child(musicCover, 0);
    if (imgArt) {
        lv_img_set_src(imgArt, imgPath);
    }

    // 2. 更新歌名
    // 確保索引在範圍內 (假設你有 5 首歌)
    if (songIndex >= 1 && songIndex <= 5) {
        lv_label_set_text(labelSongTitle, songTitles[songIndex]);
        
        // 也可以順便更新歌手名稱 (如果有需要，可以建立另一個陣列)
        // lv_label_set_text(labelArtist, "Unknown Artist");
    }
}
void UIManager::showQRCode(const char* text) {
    lv_obj_clean(qrContainer);

    lv_obj_t *qr = lv_qrcode_create(qrContainer, 180, lv_color_black(), lv_color_white());
    lv_qrcode_update(qr, text, strlen(text));
    lv_obj_center(qr);

    lv_obj_add_flag(progressBar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(labelTitle, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(qrContainer, LV_OBJ_FLAG_HIDDEN);
}
///
void UIManager::labelStatusUpdate(const char* text) {
    lv_label_set_text(labelStatus, text);
}
