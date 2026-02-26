#ifndef SPACEINVADERS_H
#define SPACEINVADERS_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

#define INV_COLS 8
#define INV_ROWS 3
#define INV_W 7
#define INV_H 5
#define INV_BULLETS 3
struct Bullet {
  float x, y;
  bool on;
};

void game_spaceinvaders() {
  bool inv[INV_ROWS][INV_COLS];
  for (int r = 0; r < INV_ROWS; r++)
    for (int c = 0; c < INV_COLS; c++)
      inv[r][c] = true;
  int invLeft = INV_ROWS * INV_COLS;
  int gridX = 4, gridY = 4;
  int8_t gridDX = 1;
  uint32_t lastInvMove = 0;
  uint16_t invSpeed = 500;
  int shipX = SCREEN_W / 2 - 4;
  const int shipY = SCREEN_H - 10;
  Bullet pb[INV_BULLETS], eb[INV_BULLETS];
  for (auto &b : pb)
    b.on = false;
  for (auto &b : eb)
    b.on = false;
  uint32_t lastShoot = 0, lastEnemyShoot = 0;
  uint16_t score = 0;
  uint8_t wave = 1;
  uint32_t lastFrame = millis();
  bool animFrame = false;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("INVADERS", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("L/R = Move (Auto-fire)", 40);
  centreStr("Destroy them all!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    lastFrame = now;
    animFrame = (now / 300) % 2;
    if (btnHeld(BTN_LEFT))
      shipX = max(0, shipX - 3);
    if (btnHeld(BTN_RIGHT))
      shipX = min(SCREEN_W - 10, shipX + 3);
    // Autofire
    if (now - lastShoot > 450) {
      for (auto &b : pb)
        if (!b.on) {
          b.x = shipX + 4;
          b.y = shipY - 1;
          b.on = true;
          beep(1500, 15);
          break;
        }
      lastShoot = now;
    }
    for (auto &b : pb) {
      if (!b.on)
        continue;
      b.y -= 4.0f * dt;
      if (b.y < 0) {
        b.on = false;
        continue;
      }
      int bc = (int(b.x) - gridX) / (INV_W + 2),
          br = (int(b.y) - gridY) / (INV_H + 3);
      if (bc >= 0 && bc < INV_COLS && br >= 0 && br < INV_ROWS && inv[br][bc]) {
        int ix = gridX + bc * (INV_W + 2), iy = gridY + br * (INV_H + 3);
        if (b.x >= ix && b.x <= ix + INV_W && b.y >= iy && b.y <= iy + INV_H) {
          inv[br][bc] = false;
          invLeft--;
          score += (INV_ROWS - br) * 10;
          b.on = false;
          beep(800 - br * 100, 40);
        }
      }
    }
    if (invLeft > 0 && now - lastEnemyShoot > max(400U, 1200U - score * 2)) {
      lastEnemyShoot = now;
      int tries = 20;
      while (tries--) {
        int c = random(0, INV_COLS);
        for (int r = INV_ROWS - 1; r >= 0; r--) {
          if (inv[r][c]) {
            for (auto &b : eb)
              if (!b.on) {
                b.x = gridX + c * (INV_W + 2) + INV_W / 2;
                b.y = gridY + r * (INV_H + 3) + INV_H;
                b.on = true;
                break;
              }
            goto shot_done;
          }
        }
      }
    shot_done:;
    }
    for (auto &b : eb) {
      if (!b.on)
        continue;
      b.y += 3.0f * dt;
      if (b.y > SCREEN_H) {
        b.on = false;
        continue;
      }
      if (b.x >= shipX && b.x <= shipX + 10 && b.y >= shipY &&
          b.y <= shipY + 7) {
        gameOver(score);
        return;
      }
    }
    if (now - lastInvMove > invSpeed) {
      lastInvMove = now;
      gridX += gridDX;
      int leftC = INV_COLS, rightC = -1;
      for (int r = 0; r < INV_ROWS; r++)
        for (int c = 0; c < INV_COLS; c++)
          if (inv[r][c]) {
            leftC = min(leftC, c);
            rightC = max(rightC, c);
          }
      if (gridX + rightC * (INV_W + 2) + INV_W >= SCREEN_W - 2 ||
          gridX + leftC * (INV_W + 2) <= 2) {
        gridDX = -gridDX;
        gridY += 3;
      }
      if (gridY + INV_ROWS * (INV_H + 3) >= shipY - 2) {
        gameOver(score);
        return;
      }
    }
    if (invLeft == 0) {
      wave++;
      invSpeed = (uint16_t)max(80, (int)invSpeed - 60);
      for (int r = 0; r < INV_ROWS; r++)
        for (int c = 0; c < INV_COLS; c++)
          inv[r][c] = true;
      invLeft = INV_ROWS * INV_COLS;
      gridX = 4;
      gridY = 4;
      gridDX = 1;
      beep(1760, 100);
      delay(110);
      beep(2093, 200);
      delay(400);
    }
    u8g2.clearBuffer();
    for (int r = 0; r < INV_ROWS; r++) {
      for (int c = 0; c < INV_COLS; c++) {
        if (!inv[r][c])
          continue;
        int ix = gridX + c * (INV_W + 2), iy = gridY + r * (INV_H + 3);
        if (r == 0) {
          u8g2.drawBox(ix + 1, iy, 5, 2);
          u8g2.drawBox(ix, iy + 2, 7, 2);
          if (animFrame) {
            u8g2.drawPixel(ix, iy + 4);
            u8g2.drawPixel(ix + 6, iy + 4);
          } else {
            u8g2.drawPixel(ix + 1, iy + 4);
            u8g2.drawPixel(ix + 5, iy + 4);
          }
        } else if (r == 1) {
          u8g2.drawBox(ix + 1, iy + 1, 5, 3);
          u8g2.drawPixel(ix + 1, iy);
          u8g2.drawPixel(ix + 5, iy);
          if (animFrame) {
            u8g2.drawPixel(ix, iy + 2);
            u8g2.drawPixel(ix + 6, iy + 2);
          } else {
            u8g2.drawPixel(ix, iy + 3);
            u8g2.drawPixel(ix + 6, iy + 3);
          }
        } else {
          u8g2.drawBox(ix + 2, iy, 3, 4);
          u8g2.drawPixel(ix + 1, iy + 1);
          u8g2.drawPixel(ix + 5, iy + 1);
          if (animFrame) {
            u8g2.drawPixel(ix, iy + 4);
            u8g2.drawPixel(ix + 3, iy + 4);
            u8g2.drawPixel(ix + 6, iy + 4);
          } else {
            u8g2.drawPixel(ix + 1, iy + 4);
            u8g2.drawPixel(ix + 5, iy + 4);
          }
        }
      }
    }
    u8g2.drawBox(shipX + 3, shipY, 4, 2);
    u8g2.drawBox(shipX + 1, shipY + 2, 8, 3);
    u8g2.drawBox(shipX, shipY + 4, 10, 3);
    for (auto &b : pb)
      if (b.on)
        u8g2.drawBox((int)b.x, (int)b.y, 1, 4);
    for (auto &b : eb)
      if (b.on) {
        u8g2.drawPixel((int)b.x, (int)b.y);
        u8g2.drawPixel((int)b.x, (int)b.y + 2);
      }
    u8g2.drawHLine(0, SCREEN_H - 1, SCREEN_W);
    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[8];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 8, sc);
    char wv[8];
    snprintf(wv, sizeof(wv), "W%u", wave);
    u8g2.drawStr(SCREEN_W - 18, 8, wv);
    u8g2.sendBuffer();
    int32_t wait = 20 - (int32_t)(millis() - now);
    if (wait > 0)
      delay(wait);
  }
}

#endif
