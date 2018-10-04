#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>
#endif

//Bloom関連ピン
#define PIN_LED   6
#define PIN_BTN   7
#define PIN_A3    A3
#define PIN_A4    A4

#define LED_NUM       (40) // LEDの数
#define LED_MAX_SAT   80  // 最大彩度

// ボタン状態
#define PRESS_NONE 0
#define PRESS_NORMAL 1
#define PRESS_LONG 2

#define LONG_PRESS_MS 2000 // 長押し検知時間

// ガンマ補正データ
const uint8_t PROGMEM gamma8[] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
  10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
  17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
  25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
  37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
  51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
  69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
  90, 92, 93, 95, 96, 98, 99, 101, 102, 104, 105, 107, 109, 110, 112, 114,
  115, 117, 119, 120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142,
  144, 146, 148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175,
  177, 180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
  215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252, 255
};

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_NUM, PIN_LED, NEO_GRB + NEO_KHZ800);

void hsv2rgb(float h, float s, float v, uint8_t colors[3]) {
  int i;
  float f, p, q, t;

  h = max(0.0, min(360.0, h));
  s = max(0.0, min(100.0, s));
  v = max(0.0, min(100.0, v));

  s /= 100;
  v /= 100;

  if (s == 0) {
    // Achromatic (grey)
    colors[0] = colors[1] = colors[2] = round(v * 255);
    return;
  }

  h /= 60; // sector 0 to 5
  i = floor(h);
  f = h - i; // factorial part of h
  p = v * (1 - s);
  q = v * (1 - s * f);
  t = v * (1 - s * (1 - f));
  switch (i) {
    case 0:
      colors[0] = round(255 * v);
      colors[1] = round(255 * t);
      colors[2] = round(255 * p);
      break;
    case 1:
      colors[0] = round(255 * q);
      colors[1] = round(255 * v);
      colors[2] = round(255 * p);
      break;
    case 2:
      colors[0] = round(255 * p);
      colors[1] = round(255 * v);
      colors[2] = round(255 * t);
      break;
    case 3:
      colors[0] = round(255 * p);
      colors[1] = round(255 * q);
      colors[2] = round(255 * v);
      break;
    case 4:
      colors[0] = round(255 * t);
      colors[1] = round(255 * p);
      colors[2] = round(255 * v);
      break;
    default: // case 5:
      colors[0] = round(255 * v);
      colors[1] = round(255 * p);
      colors[2] = round(255 * q);
  }
}

void setLEDColor(float hue, float sat, float val) {
  uint8_t rgb_colors[3];

  //getRGB(hue, sat, val, rgb_colors); // converts HSB to RGB
  hsv2rgb(hue, sat, val, rgb_colors);
  for (uint8_t i = 0; i < LED_NUM; i++) {
    strip.setPixelColor(i,
                        pgm_read_byte(&gamma8[rgb_colors[0]]),
                        pgm_read_byte(&gamma8[rgb_colors[1]]),
                        pgm_read_byte(&gamma8[rgb_colors[2]]));
  }
  strip.show();
}

void setup() {
  pinMode(PIN_BTN, INPUT);
  
  strip.begin();
  strip.clear();
  strip.show();

  // 白く光る
  for (float count = 0; count <= 100.0F; count += 1.0F) {
    setLEDColor(0, 0, count);
    delay(10);
  }
  delay(500);

  for (float count = 0; count <= LED_MAX_SAT; count += 1.0F) {
    setLEDColor(0, count, 100);
    delay(5);
  }
  delay(2000);
}

// ボタン処理
int buttonTask() {
  static long pressTime;
  static bool isPress = false;
  static bool oldState = HIGH;
  bool btnState,toggled = false;
  
  btnState = digitalRead(PIN_BTN);
  if(btnState != oldState){
    oldState = btnState;
    if (btnState == LOW) {
      if (!isPress) {
        pressTime = millis();
        isPress = true;
      }
    } else {
      if (isPress) {
        isPress = false;
        return PRESS_NORMAL;
      }
    }
  }else if (isPress) {
    if (abs(millis() - pressTime) >= LONG_PRESS_MS) {
      isPress = false;
      return PRESS_LONG;
    }
  }
  return PRESS_NONE;
}

void loop()
{
  // 時間関係
  static float nowHue = 0;
  static long oldTime;
  static bool nextFlag = false; // 次回色変更フラグ
  long diff;
  int btnState;

  if (!nextFlag) {
    for (int count = 1; count <= (LED_MAX_SAT); count++) {
      setLEDColor(nowHue, LED_MAX_SAT - ((LED_MAX_SAT / 2) - abs((LED_MAX_SAT / 2) - count)), 100);
      delay(50);
      btnState = buttonTask();
      if (btnState != PRESS_NONE) {
        nextFlag = true;
        return;
      }
    }
  } else {
    nextFlag = false;
    //白になる
    for (int count = 1; count <= LED_MAX_SAT; count++) {
      setLEDColor(nowHue, LED_MAX_SAT - count, 100);
      delay(10);
      btnState = buttonTask();
      if (btnState != PRESS_NONE) {
        nextFlag = true;
        return;
      }
    }

    //45度移動
    nowHue += 45;
    if (nowHue >= 360) {
      nowHue = 0;
    }
    delay(200);

    //LED点灯
    for (int count = 1; count <= LED_MAX_SAT; count++) {
      setLEDColor(nowHue, count, 100);
      delay(10);
      btnState = buttonTask();
      if (btnState != PRESS_NONE) {
        nextFlag = true;
        return;
      }
    }
  }
}
