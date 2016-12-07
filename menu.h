#ifndef MENU_H
#define MENU_H

#include "global.h"

extern int16_t cameraX;

namespace Menu
{
  void showTitle();
  void showStageIntro();
  void showGameOver();
  void loop();

  void drawNumber(int16_t x, int16_t y, uint16_t value, uint8_t zeroPad = 0);
}

#endif


