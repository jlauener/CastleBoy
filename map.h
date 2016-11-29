#ifndef MAP_H
#define MAP_H

#include "global.h"

namespace Map
{
void init(const uint8_t* data);
bool collide(int16_t x, int16_t y, const Rect& hitbox);
void draw();
int16_t width();
}

#endif


