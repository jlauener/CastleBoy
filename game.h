#ifndef GAME_H
#define GAME_H

#include "global.h"

extern int16_t cameraX;

namespace Game
{
  void init();
  void loop();
  void shake(uint8_t duration, int8_t strenght);
}

#endif


