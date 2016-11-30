#ifndef CANDLE_H
#define CANDLE_H

#include "global.h"

struct Entity
{
  uint8_t type;
  int16_t x;
  int16_t y;
  // TODO use bitmask to pack crap together
  bool active;
  bool alive;
  uint8_t frame;
  uint8_t counter;
  int8_t dir; // FIXME
};

namespace Entities
{
  void init();
  void add(uint8_t type, int16_t x, int16_t y);
  void update();
  void attack(int16_t x, int16_t y, int16_t w);
  Entity* collide(int16_t x, int16_t y, const Rect& hitbox);
  void draw();
}


#endif
