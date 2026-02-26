#ifndef MAZERUNNER_H
#define MAZERUNNER_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);

#define MZ_COLS 16
#define MZ_ROWS 8
#define MZ_CS 7

const uint8_t MAZE[MZ_ROWS][MZ_COLS] PROGMEM = {
    {11, 11, 9, 5, 5, 5, 5, 3, 9, 5, 5, 5, 5, 3, 9, 3},
    {10, 12, 4, 5, 7, 9, 3, 12, 4, 5, 7, 9, 3, 10, 10, 10},
    {12, 5, 5, 5, 3, 10, 12, 3, 9, 5, 3, 10, 10, 12, 2, 10},
    {13, 5, 5, 3, 12, 6, 11, 12, 6, 11, 12, 6, 12, 3, 14, 10},
    {9, 5, 3, 8, 7, 9, 4, 5, 1, 4, 1, 5, 7, 12, 5, 2},
    {10, 13, 4, 2, 9, 6, 9, 3, 10, 9, 6, 9, 5, 5, 5, 6},
    {8, 5, 3, 14, 10, 13, 6, 10, 10, 12, 7, 12, 5, 5, 5, 3},
    {12, 7, 12, 5, 4, 5, 5, 6, 12, 5, 5, 5, 5, 5, 5, 6}};

void game_maze() {
  int px = 0, py = 0, ex = MZ_COLS - 1, ey = MZ_ROWS - 1;
  uint32_t startTime = millis(), lastMove = 0;
  uint16_t moveDelay = 160;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("MAZE", 22);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("Reach the E xit!", 38);
  centreStr("Use buttons to move", 52);
  u8g2.sendBuffer();
  delay(1400);
  waitRelease();

  while (true) {
    uint32_t now = millis();
    if (now - lastMove > moveDelay) {
      uint8_t walls = pgm_read_byte(&MAZE[py][px]);
      bool moved = false;
      if (btnHeld(BTN_UP) && !(walls & 1) && py > 0) {
        py--;
        moved = true;
      }
      if (btnHeld(BTN_RIGHT) && !(walls & 2) && px < MZ_COLS - 1) {
        px++;
        moved = true;
      }
      if (btnHeld(BTN_DOWN) && !(walls & 4) && py < MZ_ROWS - 1) {
        py++;
        moved = true;
      }
      if (btnHeld(BTN_LEFT) && !(walls & 8) && px > 0) {
        px--;
        moved = true;
      }
      if (moved) {
        beep(900, 15);
        lastMove = now;
      }
    }
    if (px == ex && py == ey) {
      uint16_t secs = (millis() - startTime) / 1000;
      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_ncenB10_tr);
      centreStr("ESCAPED!", 26);
      u8g2.setFont(u8g2_font_6x10_tr);
      char buf[24];
      snprintf(buf, sizeof(buf), "Time: %us", secs);
      centreStr(buf, 42);
      centreStr("Press any button", 58);
      u8g2.sendBuffer();
      beep(1047, 80);
      delay(90);
      beep(1319, 80);
      delay(90);
      beep(1568, 200);
      delay(400);
      waitRelease();
      while (!btnHeld(BTN_UP) && !btnHeld(BTN_DOWN) && !btnHeld(BTN_LEFT) &&
             !btnHeld(BTN_RIGHT))
        delay(15);
      waitRelease();
      return;
    }
    const int OX = 8, OY = 4;
    u8g2.clearBuffer();
    for (int r = 0; r < MZ_ROWS; r++) {
      for (int c = 0; c < MZ_COLS; c++) {
        uint8_t w = pgm_read_byte(&MAZE[r][c]);
        int cx = OX + c * MZ_CS, cy = OY + r * MZ_CS;
        if (w & 1)
          u8g2.drawHLine(cx, cy, MZ_CS + 1);
        if (w & 2)
          u8g2.drawVLine(cx + MZ_CS, cy, MZ_CS + 1);
        if (w & 4)
          u8g2.drawHLine(cx, cy + MZ_CS, MZ_CS + 1);
        if (w & 8)
          u8g2.drawVLine(cx, cy, MZ_CS + 1);
      }
    }
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(OX + ex * MZ_CS + 1, OY + ey * MZ_CS + 6, "E");
    u8g2.drawDisc(OX + px * MZ_CS + MZ_CS / 2, OY + py * MZ_CS + MZ_CS / 2, 2);
    char tbuf[8];
    itoa((millis() - startTime) / 1000, tbuf, 10);
    u8g2.drawStr(SCREEN_W - u8g2.getStrWidth(tbuf) - 1, 8, tbuf);
    u8g2.sendBuffer();
    delay(20);
  }
}

#endif
