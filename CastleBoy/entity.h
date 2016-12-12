#ifndef CANDLE_H
#define CANDLE_H

#include "global.h"

#define FLAG_ACTIVE 0x80
#define FLAG_ALIVE 0x40
#define FLAG_MISC 0x20

struct Entity
{
  uint8_t type;
  Vec pos;
  uint8_t hp;

  // xxxxxxxx
  // ||
  // |+- alive
  // +-- active
  uint8_t state;
  
  uint8_t frame;
  uint8_t counter;
};

namespace Entities
{
  void init();
  void add(uint8_t type, uint8_t tileX, uint8_t tileY);
  void update();
  bool damage(int16_t x, int8_t y, uint8_t width, uint8_t height, uint8_t value);
  Entity* collide(int16_t x, int8_t y, uint8_t width, uint8_t height);
  void draw();
}


#endif
