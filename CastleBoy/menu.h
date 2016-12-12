#ifndef MENU_H
#define MENU_H

#include "global.h"

namespace Menu
{
  extern uint8_t life;
  extern uint16_t timeLeft;
  extern uint8_t stageIndex;

  //void playMusic(const uint16_t* music_ = NULL);
  //void updateMusic();
  void showTitle();
  void onStageFinished();
  void onPlayerDie();
  void loop();
}

#endif


