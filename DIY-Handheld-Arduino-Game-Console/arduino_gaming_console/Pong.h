#ifndef PONG_H
#define PONG_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);

void game_pong() {
  const int PAD_H = 14;
  const int PAD_W = 3;
  const int WIN = 7;

  float bx = 64, by = 32;
  float vx = 2.5f, vy = 1.8f;
  int pY = 25, cY = 25;
  int pScore = 0, cScore = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("PONG", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("UP/DOWN = move", 40);
  centreStr("First to 7 wins!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    if (btnHeld(BTN_UP))
      pY = max(0, pY - 3);
    if (btnHeld(BTN_DOWN))
      pY = min(SCREEN_H - PAD_H, pY + 3);

    int mid = cY + PAD_H / 2;
    if (mid < (int)by - 1)
      cY = min(SCREEN_H - PAD_H, cY + 2);
    if (mid > (int)by + 1)
      cY = max(0, cY - 2);

    bx += vx;
    by += vy;

    if (by <= 0) {
      vy = fabsf(vy);
      by = 0;
      beep(500, 15);
    }
    if (by >= SCREEN_H - 3) {
      vy = -fabsf(vy);
      by = SCREEN_H - 3;
      beep(500, 15);
    }

    // Player Paddle Collision
    if (vx < 0 && bx <= 4 + PAD_W && bx >= 4 && by + 2 >= pY &&
        by <= pY + PAD_H) {
      vx = fabsf(vx) * 1.05f;
      vy += ((by - (pY + PAD_H / 2.0f)) / (PAD_H / 2.0f)) * 1.5f;
      vy = constrain(vy, -4.5f, 4.5f);
      vx = min(vx, 5.0f);
      bx = 4 + PAD_W;
      beep(1000, 20);
    }

    if (bx >= 121 - PAD_W && bx <= 122 && by + 2 >= cY && by <= cY + PAD_H) {
      vx = -fabsf(vx) * 1.05f;
      vy += ((by - (cY + PAD_H / 2.0f)) / (PAD_H / 2.0f)) * 1.2f;
      vy = constrain(vy, -4.0f, 4.0f);
      vx = max(vx, -5.0f);
      bx = 121 - PAD_W - 1;
      beep(900, 20);
    }

    if (bx < 0) {
      cScore++;
      beep(200, 250);
      bx = 64;
      by = 32;
      vx = 2.5f;
      vy = 1.8f;
      pY = 25;
      cY = 25;
      delay(600);
    }
    if (bx > SCREEN_W) {
      pScore++;
      beep(1400, 100);
      bx = 64;
      by = 32;
      vx = -2.5f;
      vy = 1.8f;
      pY = 25;
      cY = 25;
      delay(600);
    }

    if (pScore >= WIN || cScore >= WIN) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr(pScore >= WIN ? "YOU WIN!" : "CPU WINS", 28);
      u8g2.setFont(u8g2_font_6x10_tr);
      char buf[20];
      snprintf(buf, sizeof(buf), "%d  :  %d", pScore, cScore);
      centreStr(buf, 44);
      centreStr("Press any button", 58);
      u8g2.sendBuffer();
      if (pScore >= WIN) {
        beep(1760, 100);
        delay(110);
        beep(1760, 100);
        delay(110);
        beep(2093, 300);
      } else {
        beep(300, 200);
      }
      delay(400);
      waitRelease();
      while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
             !btnHeld(BTN_RIGHT))
        delay(15);
      waitRelease();
      return;
    }

    u8g2.clearBuffer();
    for (int y = 0; y < SCREEN_H; y += 6)
      u8g2.drawPixel(63, y);
    u8g2.drawBox(4, pY, PAD_W, PAD_H);
    u8g2.drawBox(121, cY, PAD_W, PAD_H);
    u8g2.drawBox((int)bx, (int)by, 3, 3);

    u8g2.setFont(u8g2_font_6x10_tr);
    char ps[4], cs[4];
    itoa(pScore, ps, 10);
    itoa(cScore, cs, 10);
    u8g2.drawStr(42, 10, ps);
    u8g2.drawStr(78, 10, cs);
    u8g2.sendBuffer();
    delay(16);
  }
}

#endif
