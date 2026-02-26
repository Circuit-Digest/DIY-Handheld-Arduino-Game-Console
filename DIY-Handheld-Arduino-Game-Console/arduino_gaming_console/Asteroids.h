#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

#define MAX_AST 8
struct Rock {
  float x, y, spd;
  uint8_t w, h;
  bool on;
};

void game_asteroids() {
  const int SHIP_W = 9, SHIP_H = 8;
  int shipY = SCREEN_H / 2 - SHIP_H / 2;

  Rock rocks[MAX_AST];
  for (auto &r : rocks)
    r.on = false;

  uint32_t score = 0;
  uint32_t lastSpawn = 0;
  uint32_t lastFrame = millis();
  uint32_t spawnGap = 900;

  const uint8_t starX[8] = {20, 45, 70, 95, 110, 35, 60, 85};
  const uint8_t starY[8] = {8, 24, 12, 40, 55, 50, 36, 20};

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("ASTEROIDS", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("UP/DOWN to dodge", 40);
  centreStr("Survive as long", 52);
  centreStr("as you can!", 62);
  u8g2.sendBuffer();
  delay(1600);
  waitRelease();

  while (true) {
    uint32_t now = millis();
    float dt = (now - lastFrame) / 30.0f;
    lastFrame = now;

    if (btnHeld(BTN_UP))
      shipY = max(0, shipY - 3);
    if (btnHeld(BTN_DOWN))
      shipY = min(SCREEN_H - SHIP_H, shipY + 3);

    score++;
    spawnGap = max(300UL, 900UL - score / 100);

    if (now - lastSpawn > spawnGap) {
      lastSpawn = now;
      for (auto &r : rocks) {
        if (!r.on) {
          r.x = SCREEN_W + 4;
          r.y = random(2, SCREEN_H - 14);
          r.w = random(5, 13);
          r.h = random(5, 11);
          r.spd = random(18, 40) / 10.0f;
          r.on = true;
          break;
        }
      }
    }

    for (auto &r : rocks) {
      if (!r.on)
        continue;
      r.x -= r.spd * dt;
      if (r.x + r.w < 0) {
        r.on = false;
        continue;
      }
      if (r.x < 2 + SHIP_W && r.x + r.w > 2 && r.y < shipY + SHIP_H &&
          r.y + r.h > shipY) {
        gameOver(score / 10);
        return;
      }
    }

    u8g2.clearBuffer();
    for (int i = 0; i < 8; i++)
      u8g2.drawPixel(starX[i], starY[i]);
    u8g2.drawLine(2, shipY + 4, 10, shipY);
    u8g2.drawLine(2, shipY + 4, 10, shipY + 8);
    u8g2.drawLine(10, shipY, 10, shipY + 8);
    if ((now / 120) % 2 == 0) {
      u8g2.drawLine(0, shipY + 3, 2, shipY + 4);
      u8g2.drawLine(0, shipY + 5, 2, shipY + 4);
    } else {
      u8g2.drawPixel(1, shipY + 4);
    }
    for (auto &r : rocks) {
      if (!r.on)
        continue;
      int rx = (int)r.x, ry = (int)r.y;
      u8g2.drawFrame(rx, ry, r.w, r.h);
      u8g2.drawPixel(rx, ry);
      u8g2.drawPixel(rx + r.w - 1, ry);
      u8g2.drawPixel(rx, ry + r.h - 1);
      u8g2.drawPixel(rx + r.w - 1, ry + r.h - 1);
    }
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    ltoa(score / 10, sc, 10);
    uint8_t sw = u8g2.getStrWidth(sc);
    u8g2.drawStr(SCREEN_W - sw - 2, 8, sc);
    u8g2.sendBuffer();
    int32_t wait = 33 - (int32_t)(millis() - now);
    if (wait > 0)
      delay(wait);
  }
}

#endif
