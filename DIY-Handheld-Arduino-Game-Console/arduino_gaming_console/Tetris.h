#ifndef TETRIS_H
#define TETRIS_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

#define TT_COLS 8
#define TT_ROWS 13
#define TT_SZ 5
#define TT_OX 44
#define TT_OY 2

struct TetPiece {
  int8_t x, y;
  uint8_t type;
  int8_t cells[4][2];
  void load(uint8_t t);
  void getAbs(int8_t out[4][2]) const {
    for (int i = 0; i < 4; i++) {
      out[i][0] = x + cells[i][0];
      out[i][1] = y + cells[i][1];
    }
  }
};

const int8_t PIECES[7][4][2] PROGMEM = {
    {{0, 0}, {1, 0}, {2, 0}, {3, 0}}, // I
    {{0, 0}, {0, 1}, {1, 1}, {2, 1}}, // J
    {{2, 0}, {0, 1}, {1, 1}, {2, 1}}, // L
    {{0, 0}, {1, 0}, {1, 1}, {2, 1}}, // S
    {{1, 0}, {2, 0}, {0, 1}, {1, 1}}, // Z
    {{1, 0}, {0, 1}, {1, 1}, {2, 1}}, // T
    {{0, 0}, {1, 0}, {0, 1}, {1, 1}}, // O
};

uint8_t board[TT_ROWS][TT_COLS];

void TetPiece::load(uint8_t t) {
  type = t;
  for (int i = 0; i < 4; i++) {
    cells[i][0] = pgm_read_byte(&PIECES[t][i][0]);
    cells[i][1] = pgm_read_byte(&PIECES[t][i][1]);
  }
}

bool ttFits(const TetPiece &p, int dx, int dy) {
  for (int i = 0; i < 4; i++) {
    int nx = p.x + p.cells[i][0] + dx;
    int ny = p.y + p.cells[i][1] + dy;
    if (nx < 0 || nx >= TT_COLS || ny >= TT_ROWS)
      return false;
    if (ny >= 0 && board[ny][nx])
      return false;
  }
  return true;
}

void ttRotate(TetPiece &p) {
  int8_t tmp[4][2];
  for (int i = 0; i < 4; i++) {
    int8_t rx = p.cells[i][0] - p.cells[0][0];
    int8_t ry = p.cells[i][1] - p.cells[0][1];
    tmp[i][0] = p.cells[0][0] - ry;
    tmp[i][1] = p.cells[0][1] + rx;
  }
  int8_t saved[4][2];
  memcpy(saved, p.cells, sizeof(saved));
  memcpy(p.cells, tmp, sizeof(tmp));
  if (!ttFits(p, 0, 0))
    memcpy(p.cells, saved, sizeof(saved));
}

void game_tetris() {
  memset(board, 0, sizeof(board));
  uint16_t score = 0;
  uint8_t level = 1;
  uint32_t dropInterval = 600, lastDrop = millis(), lastMove = 0;
  TetPiece cur, next;
  cur.load(random(0, 7));
  cur.x = TT_COLS / 2 - 1;
  cur.y = 0;
  next.load(random(0, 7));
  next.x = TT_COLS / 2 - 1;
  next.y = 0;

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("TETRIS", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("L/R=move  UP=rot", 40);
  centreStr("DN=fast drop", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    uint32_t now = millis();
    if (now - lastMove > 130) {
      if (btnHeld(BTN_LEFT) && ttFits(cur, -1, 0)) {
        cur.x--;
        lastMove = now;
        beep(700, 15);
      }
      if (btnHeld(BTN_RIGHT) && ttFits(cur, 1, 0)) {
        cur.x++;
        lastMove = now;
        beep(700, 15);
      }
    }
    if (btnPressed(BTN_UP)) {
      ttRotate(cur);
      beep(900, 15);
    }
    uint32_t interval = btnHeld(BTN_DOWN) ? 60 : dropInterval;
    if (now - lastDrop > interval) {
      lastDrop = now;
      if (ttFits(cur, 0, 1))
        cur.y++;
      else {
        int8_t abs[4][2];
        cur.getAbs(abs);
        for (int i = 0; i < 4; i++)
          if (abs[i][1] >= 0)
            board[abs[i][1]][abs[i][0]] = 1;
        beep(500, 20);
        int cleared = 0;
        for (int r = TT_ROWS - 1; r >= 0; r--) {
          bool full = true;
          for (int c = 0; c < TT_COLS; c++)
            if (!board[r][c]) {
              full = false;
              break;
            }
          if (full) {
            cleared++;
            for (int rr = r; rr > 0; rr--)
              memcpy(board[rr], board[rr - 1], TT_COLS);
            memset(board[0], 0, TT_COLS);
            r++;
          }
        }
        if (cleared) {
          static const uint16_t pts[5] = {0, 40, 100, 300, 1200};
          score += pts[min(cleared, 4)] * level;
          beep(1200, 40);
          delay(45);
          beep(1400, 40);
          level = 1 + score / 200;
          dropInterval = max(80U, 600U - (level - 1) * 60);
        }
        cur = next;
        cur.x = TT_COLS / 2 - 1;
        cur.y = 0;
        next.load(random(0, 7));
        if (!ttFits(cur, 0, 0)) {
          gameOver(score);
          return;
        }
      }
    }
    u8g2.clearBuffer();
    u8g2.drawFrame(TT_OX - 1, TT_OY - 1, TT_COLS * TT_SZ + 2,
                   TT_ROWS * TT_SZ + 2);
    for (int r = 0; r < TT_ROWS; r++)
      for (int c = 0; c < TT_COLS; c++)
        if (board[r][c])
          u8g2.drawBox(TT_OX + c * TT_SZ, TT_OY + r * TT_SZ, TT_SZ - 1,
                       TT_SZ - 1);
    int8_t abs[4][2];
    cur.getAbs(abs);
    for (int i = 0; i < 4; i++)
      if (abs[i][1] >= 0)
        u8g2.drawBox(TT_OX + abs[i][0] * TT_SZ, TT_OY + abs[i][1] * TT_SZ,
                     TT_SZ - 1, TT_SZ - 1);
    TetPiece ghost = cur;
    while (ttFits(ghost, 0, 1))
      ghost.y++;
    ghost.getAbs(abs);
    for (int i = 0; i < 4; i++)
      if (abs[i][1] >= 0 && abs[i][1] != cur.y + cur.cells[i][1])
        u8g2.drawFrame(TT_OX + abs[i][0] * TT_SZ + 1,
                       TT_OY + abs[i][1] * TT_SZ + 1, TT_SZ - 3, TT_SZ - 3);
    u8g2.setFont(u8g2_font_5x7_tr);
    u8g2.drawStr(1, 10, "SCR");
    char sc[8];
    itoa(score, sc, 10);
    u8g2.drawStr(1, 18, sc);
    u8g2.drawStr(1, 30, "LVL");
    char lv[4];
    itoa(level, lv, 10);
    u8g2.drawStr(1, 38, lv);
    u8g2.drawStr(1, 50, "NXT");
    int8_t nabs[4][2];
    next.getAbs(nabs);
    for (int i = 0; i < 4; i++)
      u8g2.drawBox(4 + nabs[i][0] * 4, 52 + nabs[i][1] * 4, 3, 3);
    u8g2.sendBuffer();
    int32_t wait = 16 - (int32_t)(millis() - now);
    if (wait > 0)
      delay(wait);
  }
}

#endif
