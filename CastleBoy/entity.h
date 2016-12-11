#ifndef CANDLE_H
#define CANDLE_H

#include "global.h"

struct Entity
{
  uint8_t type;
  Vec pos;
  uint8_t hp;
  // TODO use bitmask ?
  bool active;
  bool alive;
  bool flag;
  uint8_t frame;
  uint8_t counter;
  int8_t dir; // FIXME ?
};

namespace Entities
{
  void init();
  void add(uint8_t type, uint8_t tileX, uint8_t tileY);
  void update();
  uint8_t damage(int16_t x, int8_t y, uint8_t width, uint8_t height, uint8_t value);
  Entity* collide(const Vec& pos, const Box& hitbox);
  void draw();
}


#endif
