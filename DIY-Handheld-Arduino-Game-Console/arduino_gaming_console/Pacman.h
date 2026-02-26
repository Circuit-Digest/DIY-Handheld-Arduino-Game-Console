#ifndef PACMAN_H
#define PACMAN_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

#define PAC_SIZE 12
#define PAC_MAZE_W 10
#define PAC_MAZE_H 5

void game_pacman() {
  int px = 64, py = 32;
  int pdx = 0, pdy = 0;
  int score = 0;

  bool maze[PAC_MAZE_H][PAC_MAZE_W];
  for (int y = 0; y < PAC_MAZE_H; y++)
    for (int x = 0; x < PAC_MAZE_W; x++)
      maze[y][x] = true;

  int gx = 10, gy = 10;
  int gDir = 0; // 0:R, 1:L, 2:D, 3:U

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("PAC-MAN", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Use Buttons to Move", 40);
  centreStr("Eat all dots!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    uint32_t start = millis();

    if (btnHeld(BTN_UP)) {
      pdx = 0;
      pdy = -2;
    }
    if (btnHeld(BTN_DOWN)) {
      pdx = 0;
      pdy = 2;
    }
    if (btnHeld(BTN_LEFT)) {
      pdx = -2;
      pdy = 0;
    }
    if (btnHeld(BTN_RIGHT)) {
      pdx = 2;
      pdy = 0;
    }

    px += pdx;
    py += pdy;

    // Boundaries
    if (px < 4)
      px = 4;
    if (px > SCREEN_W - 4)
      px = SCREEN_W - 4;
    if (py < 4)
      py = 4;
    if (py > SCREEN_H - 4)
      py = SCREEN_H - 4;

    // Dot eating
    int mx = (px * PAC_MAZE_W) / SCREEN_W;
    int my = (py * PAC_MAZE_H) / SCREEN_H;
    if (mx >= 0 && mx < PAC_MAZE_W && my >= 0 && my < PAC_MAZE_H) {
      if (maze[my][mx]) {
        maze[my][mx] = false;
        score += 10;
        beep(1000, 10);
      }
    }

    // Ghost AI (Slower)
    static uint8_t gSpeedCount = 0;
    if (++gSpeedCount % 2 == 0) {
      if (gx < px)
        gx++;
      else if (gx > px)
        gx--;
      if (gy < py)
        gy++;
      else if (gy > py)
        gy--;
    }

    // Collision
    if (abs(px - gx) < 6 && abs(py - gy) < 6) {
      gameOver(score);
      return;
    }

    // Win check
    bool win = true;
    for (int y = 0; y < PAC_MAZE_H; y++)
      for (int x = 0; x < PAC_MAZE_W; x++)
        if (maze[y][x])
          win = false;

    if (win) {
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("YOU WIN!", 32);
      u8g2.sendBuffer();
      delay(2000);
      return;
    }

    u8g2.clearBuffer();

    // Draw dots
    for (int y = 0; y < PAC_MAZE_H; y++) {
      for (int x = 0; x < PAC_MAZE_W; x++) {
        if (maze[y][x]) {
          int dx = (x * SCREEN_W / PAC_MAZE_W) + (SCREEN_W / (PAC_MAZE_W * 2));
          int dy = (y * SCREEN_H / PAC_MAZE_H) + (SCREEN_H / (PAC_MAZE_H * 2));
          u8g2.drawPixel(dx, dy);
        }
      }
    }

    // Draw Pac
    u8g2.drawDisc(px, py, 4);
    // Draw Ghost
    u8g2.drawFrame(gx - 3, gy - 3, 7, 7);
    u8g2.drawPixel(gx - 1, gy - 1);
    u8g2.drawPixel(gx + 1, gy - 1);

    u8g2.setFont(u8g2_font_5x7_tr);
    char sc[10];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 63, sc);

    u8g2.sendBuffer();

    int32_t wait = 30 - (int32_t)(millis() - start);
    if (wait > 0)
      delay(wait);
  }
}

#endif
