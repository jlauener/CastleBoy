#ifndef MENU_H
#define MENU_H

#include "global.h"

extern int16_t cameraX;

namespace Menu
{
  extern uint8_t life;
  extern uint16_t score;
  extern uint8_t stageIndex;

  void playMusic(const uint16_t* music_ = NULL);
  void updateMusic();
  void showTitle();
  void onStageFinished();
  void onPlayerDie();
  void loop();

  void drawNumber(int16_t x, int16_t y, uint16_t value, uint8_t zeroPad = 0);
}

#endif


