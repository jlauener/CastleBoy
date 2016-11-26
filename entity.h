#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "global.h"

struct EntityData
{
  int8_t width;
  int8_t height;
  int8_t originX;
  int8_t originY;
  const uint8_t* sprite;
  uint8_t flippedFrameOffset;
};

class Entity
{
  public:
    int16_t x;
    int16_t y;
    bool flipped;

    void init(const EntityData& data, int16_t x, int16_t y, bool flipped = false);
    bool collideWithMap(int16_t x, int16_t y);
    bool moveX(int16_t dx);
    bool moveY(int16_t dy);
    void draw();
    void play(const uint8_t* anim);
    bool isPlaying()
    {
      return anim[MODE] == LOOP || animCounter > 0 || animFrame < anim[FRAME_COUNT] - 1;
    }

//  private:
    const EntityData * data;
    const uint8_t* anim;
    uint8_t animFrame;
    uint8_t animCounter;
};

#endif


