#ifndef BREAKOUT_H
#define BREAKOUT_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

#define BRK_COLS 10
#define BRK_ROWS 4
#define BRK_BW 11
#define BRK_BH 5
#define BRK_PADW 20
#define BRK_PADH 3

void game_breakout() {
  uint8_t bricks[BRK_ROWS][BRK_COLS];
  for (int r = 0; r < BRK_ROWS; r++)
    for (int c = 0; c < BRK_COLS; c++)
      bricks[r][c] = 1;
  int bricksLeft = BRK_ROWS * BRK_COLS;
  int padX = (SCREEN_W - BRK_PADW) / 2;
  const int padY = SCREEN_H - 6;
  float bx = 64, by = 48;
  float vx = 1.7f, vy = -0.5f;
  int lives = 3;
  uint16_t score = 0;
  uint32_t lastFrame = millis();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("BREAKOUT", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("LEFT/RIGHT = paddle", 40);
  centreStr("Don't drop the ball!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    uint32_t now = millis();
    float dt = (now - lastFrame) / 30.0f;
    lastFrame = now;
    if (btnHeld(BTN_LEFT))
      padX = max(0, padX - 4);
    if (btnHeld(BTN_RIGHT))
      padX = min(SCREEN_W - BRK_PADW, padX + 4);
    bx += vx * dt;
    by += vy * dt;
    if (bx <= 1) {
      vx = fabsf(vx);
      bx = 1;
      beep(600, 15);
    }
    if (bx >= SCREEN_W - 4) {
      vx = -fabsf(vx);
      bx = SCREEN_W - 4;
      beep(600, 15);
    }
    if (by <= 1) {
      vy = fabsf(vy);
      by = 1;
      beep(600, 15);
    }
    if (vy > 0 && by + 3 >= padY && by + 3 <= padY + BRK_PADH + 2 &&
        bx + 2 >= padX && bx <= padX + BRK_PADW) {
      vy = -fabsf(vy);
      float rel = ((bx + 1) - (padX + BRK_PADW / 2.0f)) / (BRK_PADW / 2.0f);
      vx = rel * 3.0f;
      if (fabsf(vx) < 0.5f)
        vx = (vx >= 0) ? 0.5f : -0.5f;
      by = padY - 3; // Push further away
      beep(900, 20);
    }
    if (by > SCREEN_H + 2) {
      lives--;
      beep(200, 300);
      if (lives <= 0) {
        gameOver(score);
        return;
      }
      bx = 64;
      by = 45;
      vx = 1.8f;
      vy = -2.2f;
      delay(600);
    }
    for (int r = 0; r < BRK_ROWS && bricksLeft > 0; r++) {
      for (int c = 0; c < BRK_COLS; c++) {
        if (!bricks[r][c])
          continue;
        int bkx = c * (BRK_BW + 1) + 1;
        int bky = r * (BRK_BH + 1) + 1;
        if (bx + 3 >= bkx && bx <= bkx + BRK_BW && by + 3 >= bky &&
            by <= bky + BRK_BH) {
          bricks[r][c] = 0;
          bricksLeft--;
          score += (BRK_ROWS - r) * 10;
          float overlapL = bx + 3 - bkx, overlapR = bkx + BRK_BW - bx,
                overlapT = by + 3 - bky, overlapB = bky + BRK_BH - by;
          float minH = min(overlapL, overlapR), minV = min(overlapT, overlapB);
          if (minH < minV)
            vx = -vx;
          else
            vy = -vy;
          beep(1200 + r * 120, 25);
          goto brick_done;
        }
      }
    }
  brick_done:
    if (bricksLeft == 0) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("YOU WIN!", 28);
      u8g2.setFont(u8g2_font_6x10_tr);
      char buf[20];
      snprintf(buf, sizeof(buf), "Score: %u", score);
      centreStr(buf, 44);
      centreStr("Press any button", 58);
      u8g2.sendBuffer();
      beep(1760, 100);
      delay(110);
      beep(1760, 100);
      delay(110);
      beep(2093, 300);
      delay(400);
      waitRelease();
      while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
             !btnHeld(BTN_RIGHT))
        delay(15);
      waitRelease();
      return;
    }
    u8g2.clearBuffer();
    for (int r = 0; r < BRK_ROWS; r++) {
      for (int c = 0; c < BRK_COLS; c++) {
        if (!bricks[r][c])
          continue;
        int bkx = c * (BRK_BW + 1) + 1, bky = r * (BRK_BH + 1) + 1;
        if (r % 2 == 0)
          u8g2.drawBox(bkx, bky, BRK_BW, BRK_BH);
        else
          u8g2.drawFrame(bkx, bky, BRK_BW, BRK_BH);
      }
    }
    u8g2.drawRBox(padX, padY, BRK_PADW, BRK_PADH, 1);
    u8g2.drawDisc((int)bx + 1, (int)by + 1, 2);
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 63, sc);
    for (int i = 0; i < lives; i++) {
      u8g2.drawPixel(SCREEN_W - 4 - i * 6, 60);
      u8g2.drawBox(SCREEN_W - 6 - i * 6, 61, 5, 3);
    }
    u8g2.sendBuffer();
    int32_t wait = 20 - (int32_t)(millis() - now);
    if (wait > 0)
      delay(wait);
  }
}

#endif
