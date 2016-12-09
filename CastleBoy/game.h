#ifndef GAME_H
#define GAME_H

#include "global.h"

namespace Game
{
  extern int16_t cameraX;
  
  void play(const uint8_t* source);
  void loop();
}

#endif


