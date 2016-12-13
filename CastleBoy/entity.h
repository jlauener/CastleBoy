#ifndef CANDLE_H
#define CANDLE_H

#include "global.h"

struct Entity
{
  uint8_t type;
  Vec pos;
  uint8_t hp;

  // xxxxxxxx
  // ||||||||
  // |||||||+- n/a
  // ||||||+-- n/a
  // |||||+--- misc2
  // ||||+---- misc1
  // |||+----- hurt
  // ||+------ alive
  // |+------- active
  // +-------- present
  uint8_t state;
  
  uint8_t frame;
  uint8_t counter;
};

namespace Entities
{
  void init();
  void add(uint8_t type, int16_t x, int8_t y);
  void update();
  bool damage(int16_t x, int8_t y, uint8_t width, uint8_t height, uint8_t value);
  bool moveCollide(int16_t x, int8_t y, const Box& hitbox);
  Entity* checkPlayer(int16_t x, int8_t y, uint8_t width, uint8_t height);
 // Entity* query(int16_t x, int8_t y, uint8_t width, uint8_t height);
 // void damage(Entity& entity, uint8_t value);
  void draw();
}


#endif
