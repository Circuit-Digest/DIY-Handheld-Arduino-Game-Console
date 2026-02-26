#ifndef FLAPPYBIRD_H
#define FLAPPYBIRD_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

void game_flappy() {
  const float GRAVITY = 0.30f;
  const float FLAP_VEL = -2.20f;
  const int GAP = 18;
  const int PIPE_W = 8;
  const int BIRD_X = 14;
  const int BIRD_W = 7;
  const int BIRD_H = 5;

  float birdY = 30, vel = 0;
  int pX[2] = {128, 192};
  int pGap[2] = {20, 35};
  int score = 0;
  bool scored[2] = {false, false};
  uint32_t lastFrame = 0;

  // Instructions
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("FLAPPY BIRD", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("ANY btn = Flap", 40);
  centreStr("Press ANY to start", 54);
  u8g2.sendBuffer();
  waitRelease();
  while (!btnPressed(BTN_UP) && !btnPressed(BTN_DOWN) &&
         !btnPressed(BTN_LEFT) && !btnPressed(BTN_RIGHT))
    delay(10);
  vel = FLAP_VEL;
  beep(900, 35);
  lastFrame = millis();

  while (true) {
    uint32_t now = millis();
    float dt = (now - lastFrame) / 30.0f;
    if (dt > 3.0f)
      dt = 3.0f;
    lastFrame = now;

    // Flap
    // Flap (Any button)
    bool anyBtn = btnPressed(BTN_UP) || btnPressed(BTN_DOWN) ||
                  btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT);
    if (anyBtn) {
      vel = FLAP_VEL;
      beep(900, 30);
    }

    vel += GRAVITY * dt;
    birdY += vel * dt;

    // Move pipes
    for (int i = 0; i < 2; i++) {
      pX[i] -= (int)(2.8f * dt);
      if (pX[i] < -PIPE_W) {
        pX[i] = SCREEN_W + random(10, 40);
        pGap[i] = random(8, SCREEN_H - GAP - 8);
        scored[i] = false;
      }
      if (!scored[i] && pX[i] + PIPE_W < BIRD_X) {
        score++;
        scored[i] = true;
        beep(1300, 25);
      }
    }

    int by = (int)birdY;

    // Ceiling / floor collision
    if (by < 0 || by + BIRD_H >= SCREEN_H) {
      gameOver(score);
      return;
    }

    // Pipe collision
    for (int i = 0; i < 2; i++) {
      if (BIRD_X + BIRD_W > pX[i] && BIRD_X < pX[i] + PIPE_W) {
        if (by < pGap[i] || by + BIRD_H > pGap[i] + GAP) {
          gameOver(score);
          return;
        }
      }
    }

    // ── Draw ──
    u8g2.clearBuffer();

    for (int i = 0; i < 2; i++) {
      u8g2.drawBox(pX[i], 0, PIPE_W, pGap[i]);
      u8g2.drawBox(pX[i] - 1, pGap[i] - 4, PIPE_W + 2, 5);
      u8g2.drawBox(pX[i], pGap[i] + GAP, PIPE_W, SCREEN_H - (pGap[i] + GAP));
      u8g2.drawBox(pX[i] - 1, pGap[i] + GAP - 1, PIPE_W + 2, 5);
    }

    u8g2.drawBox(BIRD_X, by, BIRD_W, BIRD_H);
    u8g2.setDrawColor(0);
    u8g2.drawPixel(BIRD_X + 5, by + 1);
    u8g2.setDrawColor(1);
    u8g2.drawPixel(BIRD_X + 7, by + 2);

    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(2, 10, sc);

    u8g2.sendBuffer();

    int32_t wait = 33 - (int32_t)(millis() - now);
    if (wait > 0)
      delay(wait);
  }
}

#endif
