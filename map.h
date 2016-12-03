#ifndef MAP_H
#define MAP_H

#include "global.h"

namespace Map
{
extern int16_t width;

void init(const uint8_t* data);
bool collide(int16_t x, int16_t y, const Rect& hitbox);
bool moveY(int16_t x, int16_t& y, int16_t dy, const Rect& hitbox);
void draw();
}

#endif


