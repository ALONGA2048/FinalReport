#include <lvgl.h>
#include <TFT_eSPI.h>

/* ==========================================
   1. 硬體與腳位設定 (基於你的最終修復)
   ========================================== */
TFT_eSPI tft = TFT_eSPI(); 

// 這是我們剛剛驗證成功的 "A1 換 A4" 最終方案
// Pair 1 (X軸): A2 & A0
// Pair 2 (Y軸): A3 & A4
#define TOUCH_XP 3   // A2
#define TOUCH_XM 1   // A0
#define TOUCH_YP 4   // A3
#define TOUCH_YM 11  // A4 (避開 GPIO 2)

// LVGL 緩衝區設定
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

/* ==========================================
   2. 觸控讀取函式 (手動驅動)
   ========================================== */
void restore_pins() {
  pinMode(TOUCH_XP, OUTPUT); pinMode(TOUCH_YP, OUTPUT);
  pinMode(TOUCH_XM, OUTPUT); pinMode(TOUCH_YM, OUTPUT);
  digitalWrite(TOUCH_YM, HIGH); 
  digitalWrite(TOUCH_XM, HIGH);
}

bool my_touch_read(int *x, int *y) {
  int rawX = 0, rawY = 0;

  // --- 1. 讀取 X 軸 ---
  // 驅動 {XP, XM} = {A2, A0}
  // 讀取 YP = A3
  pinMode(TOUCH_XP, OUTPUT); digitalWrite(TOUCH_XP, HIGH);
  pinMode(TOUCH_XM, OUTPUT); digitalWrite(TOUCH_XM, LOW);
  pinMode(TOUCH_YP, INPUT); 
  pinMode(TOUCH_YM, INPUT); 
  delayMicroseconds(50);
  rawX = analogRead(TOUCH_YP);

  // --- 2. 讀取 Y 軸 ---
  // 驅動 {YP, YM} = {A3, A4}
  // 讀取 XP = A2
  pinMode(TOUCH_YP, OUTPUT); digitalWrite(TOUCH_YP, HIGH);
  pinMode(TOUCH_YM, OUTPUT); digitalWrite(TOUCH_YM, LOW);
  pinMode(TOUCH_XP, INPUT); 
  pinMode(TOUCH_XM, INPUT); 
  delayMicroseconds(50);
  rawY = analogRead(TOUCH_XP);

  restore_pins();

  // 判斷是否有效按壓
  if (rawX > 100 && rawX < 4000 && rawY > 100 && rawY < 4000) {
    
    // --- 校正映射 ---
    // 這裡填入你之前測得的邊界值
    int calX = map(rawX, 350, 3600, 0, 320);
    int calY = map(rawY, 250, 3600, 0, 240); 

    // 邊界限制
    if(calX < 0) calX = 0; if(calX > 320) calX = 320;
    if(calY < 0) calY = 0; if(calY > 240) calY = 240;

    *x = calX;
    *y = calY;
    return true;
  }
  return false;
}

/* ==========================================
   3. LVGL 回調函式 (v8 格式)
   ========================================== */
// 顯示刷新 (Flush)
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

// 輸入讀取 (Input Read)
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    int touchX, touchY;
    if (my_touch_read(&touchX, &touchY)) {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchX;
        data->point.y = touchY;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

/* ==========================================
   4. 主程式 Setup & Loop
   ========================================== */
void setup()
{
    Serial.begin( 115200 );
    analogReadResolution(12);

    // 1. 初始化螢幕
    tft.begin();        
    tft.setRotation( 1 ); 

    // 2. 初始化 LVGL
    lv_init();
    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    // 3. 註冊顯示驅動 (這是 v8 的寫法)
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    // 4. 註冊觸控驅動 (這是 v8 的寫法)
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );

    // 5. 建立一個簡單的測試按鈕
    lv_obj_t * btn = lv_btn_create(lv_scr_act()); // 注意：v8 是 lv_btn_create，不是 lv_button_create
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(btn, 120, 50);

    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);
}

void loop()
{
    lv_timer_handler(); 
    delay( 5 );
}