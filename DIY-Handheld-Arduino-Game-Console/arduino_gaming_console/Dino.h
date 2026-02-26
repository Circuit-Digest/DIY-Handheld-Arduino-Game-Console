#ifndef DINO_H
#define DINO_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

void game_dino() {
  const float GRAVITY = 0.6f;
  const float JUMP_FORCE = -6.5f;
  const int DINO_X = 15;
  const int GROUND_Y = 58;

  float dinoY = GROUND_Y - 12;
  float velY = 0;
  bool isJumping = false;

  float obsX = 130;
  int obsType = 0; // 0: Small cactus, 1: Large cactus, 2: Bird
  float obsSpeed = 3.5f;

  uint16_t score = 0;
  uint32_t lastFrame = millis();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("DINO RUN", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("PRESS ANY BTN", 40);
  centreStr("Avoid obstacles!", 54);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    if (dt > 2.0f)
      dt = 2.0f;
    lastFrame = now;

    // Jump logic
    // Jump logic (Any button)
    bool anyBtn = btnPressed(BTN_UP) || btnPressed(BTN_DOWN) ||
                  btnPressed(BTN_LEFT) || btnPressed(BTN_RIGHT);
    if (anyBtn && !isJumping) {
      velY = JUMP_FORCE;
      isJumping = true;
      beep(1000, 20);
    }

    velY += GRAVITY * dt;
    dinoY += velY * dt;

    if (dinoY >= GROUND_Y - 12) {
      dinoY = GROUND_Y - 12;
      velY = 0;
      isJumping = false;
    }

    // Obstacle logic
    obsX -= obsSpeed * dt;
    if (obsX < -20) {
      obsX = 130 + random(0, 50);
      obsType = random(0, 3);
      score++;
      obsSpeed += 0.05f;
      if (score % 5 == 0)
        beep(1500, 30);
    }

    // Collision detection
    int dinoW = 10, dinoH = 12;
    int obsW, obsH, obsY;
    if (obsType == 0) {
      obsW = 6;
      obsH = 10;
      obsY = GROUND_Y - 10;
    } else if (obsType == 1) {
      obsW = 10;
      obsH = 14;
      obsY = GROUND_Y - 14;
    } else {
      obsW = 8;
      obsH = 6;
      obsY = GROUND_Y - 25;
    } // Bird

    if (DINO_X < obsX + obsW && DINO_X + dinoW > obsX && dinoY < obsY + obsH &&
        dinoY + dinoH > obsY) {
      gameOver(score);
      return;
    }

    // ── Draw ──
    u8g2.clearBuffer();

    // Ground
    u8g2.drawHLine(0, GROUND_Y, SCREEN_W);
    for (int i = 0; i < SCREEN_W; i += 10)
      u8g2.drawPixel(i + ((now / 50) % 10), GROUND_Y + 2);

    // Dino
    u8g2.drawFrame(DINO_X, (int)dinoY, 10, 12);
    u8g2.drawBox(DINO_X + 7, (int)dinoY + 2, 2, 2); // Eye
    if (!isJumping) {
      if ((now / 100) % 2 == 0)
        u8g2.drawBox(DINO_X + 2, (int)dinoY + 12, 2, 2);
      else
        u8g2.drawBox(DINO_X + 6, (int)dinoY + 12, 2, 2);
    }

    // Obstacles
    if (obsType == 0) { // Small cactus
      u8g2.drawBox((int)obsX + 2, (int)obsY, 2, 10);
      u8g2.drawPixel((int)obsX, (int)obsY + 2);
      u8g2.drawPixel((int)obsX + 4, (int)obsY + 1);
    } else if (obsType == 1) { // Large cactus
      u8g2.drawBox((int)obsX + 3, (int)obsY, 4, 14);
      u8g2.drawBox((int)obsX, (int)obsY + 4, 2, 6);
      u8g2.drawBox((int)obsX + 8, (int)obsY + 3, 2, 7);
    } else { // Bird
      if (obsX > -10 && obsX < SCREEN_W + 10) {
        u8g2.drawFrame((int)obsX, (int)obsY, 8, 4);
        if ((now / 150) % 2 == 0)
          u8g2.drawLine((int)obsX, (int)obsY, (int)obsX - 4, (int)obsY - 2);
        else
          u8g2.drawLine((int)obsX, (int)obsY + 4, (int)obsX - 4, (int)obsY + 6);
      }
    }

    // Score
    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(100, 10, sc);

    u8g2.sendBuffer();
    int32_t wait = 20 - (int32_t)(millis() - now);
    if (wait > 0)
      delay(wait);
  }
}

#endif
