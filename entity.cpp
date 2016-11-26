#include "entity.h"

#include "map.h"

void Entity::init(const EntityData& data, int16_t x, int16_t y, bool flipped)
{
  this->data = &data;
  this->x = x;
  this->y = y;
  this->flipped = flipped;
  anim = NULL;
  animCounter = 0;
}

bool Entity::collideWithMap(int16_t x, int16_t y)
{
  return Map::collide(x - data->originX, y - data->originY, data->width, data->height);
}

bool Entity::moveX(int16_t dx)
{
  if (dx != 0)
  {
    int8_t sign = dx > 0 ? 1 : -1;
    while (dx != 0)
    {
      if(collideWithMap(x + sign, y))
      {
        return true;
      }
      x += sign;
      dx -= sign;
    }
  }

  return false;
}

bool Entity::moveY(int16_t dy)
{
  if (dy != 0)
  {
    int8_t sign = dy > 0 ? 1 : -1;
    while (dy != 0)
    {
      if(collideWithMap(x, y + sign))
      {
        return true;
      }
      y += sign;
      dy -= sign;
    }
  }

  return false;
}

void Entity::play(const uint8_t* anim)
{
  if (anim != this->anim || anim[MODE] == ONE_SHOT)
  {
    this->anim = anim;
    animFrame = 0;
    animCounter = anim[FRAME_RATE];
  }
}

void Entity::draw()
{
  if (animCounter > 0)
  {
    if (--animCounter == 0)
    {
      if (animFrame < anim[FRAME_COUNT] - 1)
      {
        ++animFrame;
        animCounter = anim[FRAME_RATE];
      }
      else if (anim[MODE] == LOOP)
      {
        animFrame = 0;
        animCounter = anim[FRAME_RATE];
      }
    }
  }

  // TODO handle sprite offset (-8 and +16 are hacks)
  sprites.drawPlusMask(x - 8 - cameraX, y - 16, data->sprite, anim[FRAMES + animFrame] + (flipped ? data->flippedFrameOffset : 0));
}

