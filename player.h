#ifndef PLAYER_H
#define PLAYER_H

#include "global.h"

namespace Player
{
extern Vec pos;
extern uint8_t hp;
extern uint16_t score;
  
void init(int16_t x, int8_t y);
void update();
void draw();
}

#endif


