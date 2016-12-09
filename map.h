#ifndef MAP_H
#define MAP_H

#include "global.h"

namespace Map
{
extern uint8_t width;

void init(const uint8_t* source);
bool collide(int16_t x, int8_t y, const Box& hitbox);
bool moveY(Vec& pos, int8_t dy, const Box& hitbox);
void draw();
}

#endif


