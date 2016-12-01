#include "entity.h"

#include "assets.h"

#define DIE_ANIM_ORIGIN_X 4
#define DIE_ANIM_ORIGIN_Y 10

// TODO merge with enemy and create generic entity!
namespace
{

struct EntityData
{
  Rect hitbox;
  Point spriteOrigin;
  bool collidable; // TODO use bitmask
  const uint8_t* sprite;
};

const EntityData data[] =
{
  // candle
  {
    2, 8, // hitbox x, y
    4, 6, // hitbox width, height
    4, 10, // sprite origin x, y
    false, // false
    candle_plus_mask // sprite
  },
  // skeleton
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
    true, // collidable
    skeleton_plus_mask // sprite
  }
};

Entity entities[ENTITY_MAX];
} // unamed

void Entities::init()
{
  for (byte i = 0; i < ENTITY_MAX; i++)
  {
    entities[i].active = false;
  }
}

void Entities::add(uint8_t type, int16_t x, int16_t y)
{
  for (byte i = 0; i < ENTITY_MAX; i++)
  {
    if (!entities[i].active)
    {
      entities[i].type = type;
      entities[i].x = x;
      entities[i].y = y;
      entities[i].active = true;
      entities[i].alive = true;
      entities[i].frame = 0;
      entities[i].counter = 0;
      entities[i].dir = -1;
      return;
    }
  }

  // FIXME assert?
}


void Entities::update()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    if (entities[i].active)
    {
      if (entities[i].alive)
      {
        switch (entities[i].type)
        {
          case ENTITY_CANDLE:
            if (ab.everyXFrames(8))
            {
              ++entities[i].frame %= 2;
            }
            break;
          case ENTITY_SKELETON:
            if (ab.everyXFrames(3))
            {
              entities[i].x += entities[i].dir;
              if (++entities[i].counter == 23)
              {
                entities[i].counter = 0;
                entities[i].dir = entities[i].dir == -1 ? 1 : -1;
              }
            }
            if(ab.everyXFrames(8))
            {
              ++entities[i].frame %= 2;
            }
            break;
        }
      }
      else
      {
        if(++entities[i].counter == 8)
        {
          if (++entities[i].frame == 3)
          {
            entities[i].active = false;
          }
          entities[i].counter = 0;
        }
      }
    }
  }
}

void Entities::attack(int16_t x, int16_t y, int16_t w)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    if (entities[i].alive)
    {
      if (Util::collideHLine(x, y, w, entities[i].x, entities[i].y, data[entities[i].type].hitbox))
      {
        entities[i].alive = false;
        entities[i].frame = 0;
        entities[i].counter = 0;
      }
    }
  }
}

Entity* Entities::collide(int16_t x, int16_t y, const Rect& hitbox)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    if (entities[i].alive && data[entities[i].type].collidable)
    {
      if (Util::collide(x, y, hitbox, entities[i].x, entities[i].y, data[entities[i].type].hitbox))
      {
        return &entities[i];
      }
    }
  }
  return NULL;
}

void Entities::draw()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    if (entities[i].active)
    {
      if (entities[i].alive)
      {
        sprites.drawPlusMask(entities[i].x - data[entities[i].type].spriteOrigin.x - cameraX, entities[i].y - data[entities[i].type].spriteOrigin.y, data[entities[i].type].sprite, entities[i].frame);

#ifdef DEBUG_HITBOX
        ab.fillRect(entities[i].x - data[entities[i].type].hitbox.x - cameraX, entities[i].y - data[entities[i].type].hitbox.y, data[entities[i].type].hitbox.width, data[entities[i].type].hitbox.height);
#endif
      }
      else
      {
        sprites.drawPlusMask(entities[i].x - DIE_ANIM_ORIGIN_X - cameraX, entities[i].y - DIE_ANIM_ORIGIN_Y, entity_destroy_plus_mask, entities[i].frame);
      }
    }
  }
}
