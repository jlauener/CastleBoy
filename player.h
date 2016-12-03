#ifndef PLAYER_H
#define PLAYER_H

#include "global.h"

namespace Player
{
extern uint8_t hp;
extern uint16_t score;
  
void init(int16_t x, int16_t y);
void update();
void draw();
}

#endif


