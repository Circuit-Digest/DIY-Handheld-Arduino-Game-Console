#ifndef SNAKE_H
#define SNAKE_H

#include <Arduino.h>
#include <U8g2lib.h>

// Snake constants
#define SN_COLS 21 // grid width  (each cell = 6 px)
#define SN_ROWS 10 // grid height (each cell = 6 px)
#define SN_SZ 6
#define SN_OFFX 1
#define SN_OFFY 4
#define SN_MAXLEN 80

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

void game_snake() {
  int sx[SN_MAXLEN], sy[SN_MAXLEN];
  int len = 4;
  int dx = 1, dy = 0;
  int next_dx = 1, next_dy = 0; // Buffer for next direction
  int fx, fy;
  uint32_t lastMove = 0;
  uint16_t spd = 210; // ms per step
  uint16_t score = 0;

  // Place food avoiding snake body
  auto placeFood = [&]() {
    bool ok;
    do {
      ok = true;
      fx = random(0, SN_COLS);
      fy = random(0, SN_ROWS);
      for (int i = 0; i < len; i++)
        if (sx[i] == fx && sy[i] == fy) {
          ok = false;
          break;
        }
    } while (!ok);
  };

  for (int i = 0; i < len; i++) {
    sx[i] = len - 1 - i;
    sy[i] = SN_ROWS / 2;
  }
  placeFood();

  // Brief instructions
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("SNAKE", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Use buttons to steer", 40);
  centreStr("Eat to grow!", 54);
  u8g2.sendBuffer();
  delay(1400);
  waitRelease();

  while (true) {
    // Direction input (buffer the next move)
    // Validate against current direction (dx, dy) to prevent U-turn
    if (btnHeld(BTN_UP) && dy == 0) {
      next_dx = 0;
      next_dy = -1;
    }
    if (btnHeld(BTN_DOWN) && dy == 0) {
      next_dx = 0;
      next_dy = 1;
    }
    if (btnHeld(BTN_LEFT) && dx == 0) {
      next_dx = -1;
      next_dy = 0;
    }
    if (btnHeld(BTN_RIGHT) && dx == 0) {
      next_dx = 1;
      next_dy = 0;
    }

    uint32_t now = millis();
    if (now - lastMove < spd) {
      delay(8);
      continue;
    }
    lastMove = now;

    // Apply buffered direction
    dx = next_dx;
    dy = next_dy;

    // Next head position (wrap at borders)
    int nx = (sx[0] + dx + SN_COLS) % SN_COLS;
    int ny = (sy[0] + dy + SN_ROWS) % SN_ROWS;

    // Self-collision check
    for (int i = 1; i < len; i++) {
      if (sx[i] == nx && sy[i] == ny) {
        gameOver(score);
        return;
      }
    }

    // Shift body
    for (int i = len - 1; i > 0; i--) {
      sx[i] = sx[i - 1];
      sy[i] = sy[i - 1];
    }
    sx[0] = nx;
    sy[0] = ny;

    // Eat food?
    if (nx == fx && ny == fy) {
      score++;
      if (len < SN_MAXLEN)
        len++;
      spd = max(70, (int)spd - 7);
      beep(1400, 35);
      placeFood();
    }

    // ── Draw ──
    uint8_t ox = SN_OFFX, oy = SN_OFFY;
    u8g2.clearBuffer();

    // Playfield border
    u8g2.drawFrame(ox - 1, oy - 1, SN_COLS * SN_SZ + 2, SN_ROWS * SN_SZ + 2);

    // Snake
    for (int i = 0; i < len; i++) {
      int px = ox + sx[i] * SN_SZ;
      int py = oy + sy[i] * SN_SZ;
      if (i == 0)
        u8g2.drawBox(px, py, SN_SZ, SN_SZ); // solid head
      else
        u8g2.drawFrame(px + 1, py + 1, SN_SZ - 2, SN_SZ - 2); // hollow body
    }

    // Food (small filled square)
    u8g2.drawBox(ox + fx * SN_SZ + 1, oy + fy * SN_SZ + 1, SN_SZ - 2,
                 SN_SZ - 2);

    // Score panel (right of grid)
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(ox + SN_COLS * SN_SZ + 4, oy + 8, "SC");
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(ox + SN_COLS * SN_SZ + 4, oy + 18, sc);

    u8g2.sendBuffer();
  }
}

#endif
