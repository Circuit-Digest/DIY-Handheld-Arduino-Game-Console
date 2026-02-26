#ifndef TANK_H
#define TANK_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
extern bool btnPressed(uint8_t pin);
extern bool btnHeld(uint8_t pin);
extern void beep(uint16_t freq, uint16_t ms);
extern void waitRelease();
extern void centreStr(const char *s, uint8_t y);
extern void gameOver(uint16_t score);

#define MAX_ENEMY_TANKS 3
struct Tank {
  float x, y;
  int8_t dx, dy; // Direction: 0:UP, 1:RIGHT, 2:DOWN, 3:LEFT
  bool active;
  uint32_t lastMove;
};

struct TankBullet {
  float x, y;
  int8_t dx, dy;
  bool active;
};

void game_tank() {
  Tank player = {64, 50, 0, true, 0};
  Tank enemies[MAX_ENEMY_TANKS];
  for (int i = 0; i < MAX_ENEMY_TANKS; i++)
    enemies[i].active = false;

  TankBullet pBullet = {0, 0, 0, 0, false};
  TankBullet eBullets[MAX_ENEMY_TANKS];
  for (int i = 0; i < MAX_ENEMY_TANKS; i++)
    eBullets[i].active = false;

  uint16_t score = 0;
  uint32_t lastSpawn = 0;
  uint32_t lastFrame = millis();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  centreStr("TANK BATTLE", 24);
  u8g2.setFont(u8g2_font_6x10_tr);
  centreStr("D-pad = Move", 40);
  centreStr("LEFT = Fire", 52);
  u8g2.sendBuffer();
  delay(1500);
  waitRelease();

  while (true) {
    uint32_t now = millis();
    float dt = (now - lastFrame) / 20.0f;
    lastFrame = now;

    // Player input
    if (btnHeld(BTN_UP)) {
      player.y -= 1.5f * dt;
      player.dx = 0;
      player.dy = -1;
    } else if (btnHeld(BTN_DOWN)) {
      player.y += 1.5f * dt;
      player.dx = 0;
      player.dy = 1;
    } else if (btnHeld(BTN_LEFT)) {
      player.x -= 1.5f * dt;
      player.dx = -1;
      player.dy = 0;
    } else if (btnHeld(BTN_RIGHT)) {
      player.x += 1.5f * dt;
      player.dx = 1;
      player.dy = 0;
    }

    player.x = constrain(player.x, 0, SCREEN_W - 8);
    player.y = constrain(player.y, 0, SCREEN_H - 8);

    if (btnPressed(BTN_LEFT) && !pBullet.active) {
      pBullet.x = player.x + 3;
      pBullet.y = player.y + 3;
      pBullet.dx = player.dx;
      pBullet.dy = player.dy;
      pBullet.active = true;
      beep(1200, 20);
    }

    // Spawn enemies
    if (now - lastSpawn > 3000) {
      for (int i = 0; i < MAX_ENEMY_TANKS; i++) {
        if (!enemies[i].active) {
          enemies[i].x = random(0, SCREEN_W - 8);
          enemies[i].y = 0;
          enemies[i].dx = 0;
          enemies[i].dy = 1;
          enemies[i].active = true;
          enemies[i].lastMove = now;
          lastSpawn = now;
          break;
        }
      }
    }

    // Move enemies & enemy fire
    for (int i = 0; i < MAX_ENEMY_TANKS; i++) {
      if (!enemies[i].active)
        continue;

      if (now - enemies[i].lastMove > 1000) {
        int dir = random(0, 4);
        if (dir == 0) {
          enemies[i].dx = 0;
          enemies[i].dy = -1;
        } else if (dir == 1) {
          enemies[i].dx = 1;
          enemies[i].dy = 0;
        } else if (dir == 2) {
          enemies[i].dx = 0;
          enemies[i].dy = 1;
        } else {
          enemies[i].dx = -1;
          enemies[i].dy = 0;
        }
        enemies[i].lastMove = now;

        // Enemy fire
        if (!eBullets[i].active) {
          eBullets[i].x = enemies[i].x + 3;
          eBullets[i].y = enemies[i].y + 3;
          eBullets[i].dx = enemies[i].dx;
          eBullets[i].dy = enemies[i].dy;
          eBullets[i].active = true;
        }
      }

      enemies[i].x += enemies[i].dx * 0.8f * dt;
      enemies[i].y += enemies[i].dy * 0.8f * dt;
      enemies[i].x = constrain(enemies[i].x, 0, SCREEN_W - 8);
      enemies[i].y = constrain(enemies[i].y, 0, SCREEN_H - 8);
    }

    // Move bullets
    if (pBullet.active) {
      pBullet.x += pBullet.dx * 3.0f * dt;
      pBullet.y += pBullet.dy * 3.0f * dt;
      if (pBullet.x < 0 || pBullet.x > SCREEN_W || pBullet.y < 0 ||
          pBullet.y > SCREEN_H)
        pBullet.active = false;
    }
    for (int i = 0; i < MAX_ENEMY_TANKS; i++) {
      if (eBullets[i].active) {
        eBullets[i].x += eBullets[i].dx * 2.0f * dt;
        eBullets[i].y += eBullets[i].dy * 2.0f * dt;
        if (eBullets[i].x < 0 || eBullets[i].x > SCREEN_W ||
            eBullets[i].y < 0 || eBullets[i].y > SCREEN_H)
          eBullets[i].active = false;
      }
    }

    // Bullet collisions
    if (pBullet.active) {
      for (int i = 0; i < MAX_ENEMY_TANKS; i++) {
        if (enemies[i].active && pBullet.x > enemies[i].x &&
            pBullet.x < enemies[i].x + 8 && pBullet.y > enemies[i].y &&
            pBullet.y < enemies[i].y + 8) {
          enemies[i].active = false;
          pBullet.active = false;
          score += 50;
          beep(800, 40);
        }
      }
    }
    for (int i = 0; i < MAX_ENEMY_TANKS; i++) {
      if (eBullets[i].active && eBullets[i].x > player.x &&
          eBullets[i].x < player.x + 8 && eBullets[i].y > player.y &&
          eBullets[i].y < player.y + 8) {
        gameOver(score);
        return;
      }
    }

    // ── Draw ──
    u8g2.clearBuffer();

    // Draw Player
    u8g2.drawFrame((int)player.x, (int)player.y, 8, 8);
    if (player.dy == -1)
      u8g2.drawVLine((int)player.x + 3, (int)player.y - 2, 3);
    else if (player.dy == 1)
      u8g2.drawVLine((int)player.x + 3, (int)player.y + 7, 3);
    else if (player.dx == -1)
      u8g2.drawHLine((int)player.x - 2, (int)player.y + 3, 3);
    else
      u8g2.drawHLine((int)player.x + 7, (int)player.y + 3, 3);

    // Draw Enemies
    u8g2.setDrawColor(1);
    for (int i = 0; i < MAX_ENEMY_TANKS; i++) {
      if (enemies[i].active) {
        u8g2.drawBox((int)enemies[i].x, (int)enemies[i].y, 8, 8);
        u8g2.setDrawColor(0);
        u8g2.drawPixel((int)enemies[i].x + 3, (int)enemies[i].y + 3);
        u8g2.setDrawColor(1);
      }
    }

    // Draw Bullets
    if (pBullet.active)
      u8g2.drawDisc((int)pBullet.x, (int)pBullet.y, 1);
    for (int i = 0; i < MAX_ENEMY_TANKS; i++)
      if (eBullets[i].active)
        u8g2.drawPixel((int)eBullets[i].x, (int)eBullets[i].y);

    // Score
    u8g2.setFont(u8g2_font_6x10_tr);
    char sc[6];
    itoa(score, sc, 10);
    u8g2.drawStr(2, 10, sc);

    u8g2.sendBuffer();
    int32_t wait = 25 - (int32_t)(millis() - now);
    if (wait > 0)
      delay(wait);
  }
}

#endif
