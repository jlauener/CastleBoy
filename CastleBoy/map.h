#ifndef MAP_H
#define MAP_H

#include "global.h"

namespace Map
{
extern uint8_t width;
extern bool showBackground;

void init(const uint8_t* source);
bool collide(int16_t x, int8_t y, const Box& hitbox);
void draw();
}

#endif


