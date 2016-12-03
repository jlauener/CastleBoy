#include "entity.h"

#include "assets.h"
#include "map.h"
#include "player.h"

#define DIE_ANIM_ORIGIN_X 4
#define DIE_ANIM_ORIGIN_Y 10

// TODO merge with enemy and create generic entity!
namespace
{

struct EntityData
{
  Rect hitbox;
  Point spriteOrigin;
  uint16_t score;
  //bool collidable; // TODO use bitmask
  const uint8_t* sprite;
};

const EntityData data[] =
{
  // candle
  {
    2, 8, // hitbox x, y
    4, 6, // hitbox width, height
    4, 10, // sprite origin x, y
    SCORE_CANDLE, // score
    //false, // collidable
    entity_candle_plus_mask // sprite
  },
  // skeleton
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
    SCORE_SKELETON, // score
    // true, // collidable
    entity_skeleton_plus_mask // sprite
  },
  // coin
  {
    3, 6, // hitbox x, y
    6, 6, // hitbox width, height
    4, 8, // sprite origin x, y
    // true, // collidable
    SCORE_COIN, // score
    entity_coin_plus_mask // sprite
  },
};

Entity entities[ENTITY_MAX];
} // unamed

void Entities::init()
{
  for (byte i = 0; i < ENTITY_MAX; i++)
  {
    entities[i].active = false;
    entities[i].alive = false;
  }
}

void Entities::add(uint8_t type, int16_t x, int16_t y)
{
  for (byte i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (!entity.active)
    {
      entity.type = type;
      entity.x = x;
      entity.y = y;
      entity.active = true;
      entity.alive = true;
      entity.frame = 0;
      entity.counter = 0;
      entity.dir = -1;
      return;
    }
  }

  // FIXME assert?
}


void Entities::update()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.active)
    {
      if (entity.alive)
      {
        switch (entity.type)
        {
          case ENTITY_CANDLE:
            if (ab.everyXFrames(8))
            {
              ++entity.frame %= 2;
            }
            break;
          case ENTITY_COIN:
            Map::moveY(entity.x, entity.y, 2, data[entity.type].hitbox);
            if (ab.everyXFrames(12))
            {
              ++entity.frame %= 2;
            }
            break;
          case ENTITY_SKELETON:
            if (ab.everyXFrames(3))
            {
              entity.x += entity.dir;
              if (++entity.counter == 23)
              {
                entity.counter = 0;
                entity.dir = entity.dir == -1 ? 1 : -1;
              }
            }
            if (ab.everyXFrames(8))
            {
              ++entity.frame %= 2;
            }
            break;
        }
      }
      else
      {
        if (++entity.counter == 8)
        {
          if (++entity.frame == 3)
          {
            entity.active = false;
          }
          entity.counter = 0;
        }
      }
    }
  }
}

void Entities::attack(int16_t x, int16_t y, int16_t w)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.alive && entity.type != ENTITY_COIN)
    {
      if (Util::collideHLine(x, y, w, entity.x, entity.y, data[entity.type].hitbox))
      {
        Player::score += data[entity.type].score;
        if (entity.type == ENTITY_CANDLE)
        {
          // special case: candle spawn a coin
          entity.type = ENTITY_COIN;
          entity.alive = true;
          entity.frame = 0;
        }
        else
        {
          entity.alive = false;
          entity.frame = 0;
          entity.counter = 0;
        }
        sound.tone(NOTE_CS3, 25);
      }
    }
  }
}

Entity* Entities::collide(int16_t x, int16_t y, const Rect& hitbox)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.alive && entity.type != ENTITY_CANDLE)
    {
      if (Util::collide(x, y, hitbox, entity.x, entity.y, data[entity.type].hitbox))
      {
        if (entity.type == ENTITY_COIN)
        {
          entity.alive = false;
          entity.active = false;
          Player::score += data[entity.type].score;
          sound.tone(NOTE_CS6, 30, NOTE_CS5, 40);
        }
        else
        {
          return &entity;
        }
      }
    }
  }
  return NULL;
}

void Entities::draw()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.active)
    {
      if (entity.alive)
      {
        sprites.drawPlusMask(entity.x - data[entity.type].spriteOrigin.x - cameraX, entity.y - data[entity.type].spriteOrigin.y, data[entity.type].sprite, entity.frame);

#ifdef DEBUG_HITBOX
        ab.fillRect(entity.x - data[entity.type].hitbox.x - cameraX, entity.y - data[entity.type].hitbox.y, data[entity.type].hitbox.width, data[entity.type].hitbox.height);
#endif
      }
      else
      {
        sprites.drawPlusMask(entity.x - DIE_ANIM_ORIGIN_X - cameraX, entity.y - DIE_ANIM_ORIGIN_Y, fx_destroy_plus_mask, entity.frame);
      }
    }
  }
}
