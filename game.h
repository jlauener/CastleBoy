#ifndef GAME_H
#define GAME_H

#include "global.h"

namespace Game
{
  extern uint16_t score;
  extern int16_t cameraX;
  
  void reset();
  void play();
  
  void loop();
}

#endif


