
// 觸控線（復用 TFT 控制線）
#define TOUCH_XP 11   // LCD_D0
#define TOUCH_XM 13 // LCD_RS
#define TOUCH_YP 12   // LCD_D1
#define TOUCH_YM 14   // LCD_CS



int xMin = 4095, xMax = 0;
int yMin = 4095, yMax = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Touchpad Raw Value Test ===");
  analogReadResolution(12); // 12-bit ADC
}

void loop() {
  Serial.print("XP="); Serial.print(analogRead(TOUCH_XP));
  Serial.print(" XM="); Serial.print(analogRead(TOUCH_XM));
  Serial.print(" YP="); Serial.print(analogRead(TOUCH_YP));
  Serial.print(" YM="); Serial.println(analogRead(TOUCH_YM));
  delay(200);
}

int readX() {
  // 讀 X: 驅動 XP/XM, 讀 YP
  pinMode(TOUCH_XP, OUTPUT); digitalWrite(TOUCH_XP, HIGH);
  pinMode(TOUCH_XM, OUTPUT); digitalWrite(TOUCH_XM, LOW);
  pinMode(TOUCH_YP, INPUT);
  pinMode(TOUCH_YM, INPUT);
  delayMicroseconds(50);
  int val = analogRead(TOUCH_YP);
  restorePins();
  return val;
}

int readY() {
  // 讀 Y: 驅動 YP/YM, 讀 XP
  pinMode(TOUCH_YP, OUTPUT); digitalWrite(TOUCH_YP, HIGH);
  pinMode(TOUCH_YM, OUTPUT); digitalWrite(TOUCH_YM, LOW);
  pinMode(TOUCH_XP, INPUT);
  pinMode(TOUCH_XM, INPUT);
  delayMicroseconds(50);
  int val = analogRead(TOUCH_XP);
  restorePins();
  return val;
}

void restorePins() {
  // 將所有腳位恢復成 INPUT，避免浮動或干擾
  pinMode(TOUCH_XP, INPUT);
  pinMode(TOUCH_XM, INPUT);
  pinMode(TOUCH_YP, INPUT);
  pinMode(TOUCH_YM, INPUT);
}