#ifndef ENEMY_H
#define ENEMY_H

#include "global.h"

namespace Enemies
{
void init();
void add(uint8_t type, int16_t x, int16_t y);
void hit(int16_t x, int16_t y, int16_t w);
void update();
void draw();
}

#endif


