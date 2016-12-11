#include "entity.h"

#include "assets.h"
#include "map.h"
#include "player.h"
#include "menu.h"

#define DIE_ANIM_ORIGIN_X 4
#define DIE_ANIM_ORIGIN_Y 10

// TODO refactor damage and collide (let's wait for falling blocks)
namespace
{

struct EntityData
{
  Box hitbox;
  Point spriteOrigin;
  uint8_t hp;
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
    1, // hp
    SCORE_CANDLE, // score
    //false, // collidable
    entity_candle_plus_mask // sprite
  },
  // skeleton
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
    2, // hp
    SCORE_SKELETON, // score
    // true, // collidable
    entity_skeleton_plus_mask // sprite
  },
  // skull
  {
    2, 6, // hitbox x, y
    4, 6, // hitbox width, height
    4, 8, // sprite origin x, y
    1, // hp
    SCORE_SKULL, // score
    // true, // collidable
    entity_skull_plus_mask // sprite
  },
  // coin
  {
    3, 6, // hitbox x, y
    6, 6, // hitbox width, height
    4, 8, // sprite origin x, y
    0, // hp
    SCORE_COIN, // score
    entity_coin_plus_mask // sprite
  },
};

Entity entities[ENTITY_MAX];
} // unamed

void Entities::init()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    entities[i].active = false;
    entities[i].alive = false;
  }
}

void Entities::add(uint8_t type, uint8_t tileX, uint8_t tileY)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (!entity.active)
    {
      entity.type = type;
      entity.pos.x = tileX * TILE_WIDTH + HALF_TILE_WIDTH;
      entity.pos.y = tileY * TILE_HEIGHT + TILE_HEIGHT;
      entity.hp = data[type].hp;
      entity.active = true;
      entity.alive = true;
      entity.flag = false;
      entity.frame = 0;
      entity.counter = 0;
      entity.dir = -1;
      return;
    }
  }

  // FIXME assert?
}

inline void updateSkeleton(Entity& entity)
{
  if (ab.everyXFrames(3))
  {
    entity.pos.x += entity.dir;
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
}

// not inlining this method, we gain some bytes (?)
void updateSkull(Entity& entity)
{
  if (!entity.flag && entity.pos.x - Player::pos.x < 72)
  {
    entity.flag = true;
  }

  if (entity.flag)
  {
    if (ab.everyXFrames(2))
    {
      --entity.pos.x;
      if (entity.pos.x < -8)
      {
        entity.active = false;
      }

      entity.pos.y += ++entity.counter / 20 % 2 ? 1 : -1;
    }
    if (ab.everyXFrames(8))
    {
      ++entity.frame %= 2;
    }
  }
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
            Map::moveY(entity.pos, 2, data[entity.type].hitbox);
            if (ab.everyXFrames(12))
            {
              ++entity.frame %= 2;
            }
            break;
          case ENTITY_SKELETON:
            updateSkeleton(entity);
            break;
          case ENTITY_SKULL:
            updateSkull(entity);
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

uint8_t Entities::damage(int16_t x, int8_t y, uint8_t width, uint8_t height, uint8_t value)
{
  uint8_t hitCount = 0;
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.alive && entity.type != ENTITY_COIN)
    {
      const EntityData& entityData = data[entity.type];
      if(Util::collideRect(entity.pos.x - entityData.hitbox.x, entity.pos.y - entityData.hitbox.y, entityData.hitbox.width, entityData.hitbox.height, x, y, width, height))
      {
        ++hitCount;
        if (entity.hp <= value)
        {
          Menu::score += entityData.score;
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
          sound.tone(NOTE_CS3H, 30);
        }
        else
        {
          entity.hp -= value;
          sound.tone(NOTE_CS3H, 15);
        }
      }
    }
  }

  if(hitCount)
  {
    LOG_DEBUG(hitCount);
  }
  return hitCount;
}

Entity* Entities::collide(const Vec& pos, const Box& hitbox)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.alive && entity.type != ENTITY_CANDLE)
    {
      const Box& entityHitbox = data[entity.type].hitbox;
      if(Util::collideRect(entity.pos.x - entityHitbox.x, entity.pos.y - entityHitbox.y, entityHitbox.width, entityHitbox.height, pos.x - hitbox.x, pos.y - hitbox.y, hitbox.width, hitbox.height))
      {
        // special case: coin doesn't damage player
        if (entity.type == ENTITY_COIN)
        {
          entity.alive = false;
          entity.active = false;
          Menu::score += data[entity.type].score;
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
        sprites.drawPlusMask(entity.pos.x - data[entity.type].spriteOrigin.x - Game::cameraX, entity.pos.y - data[entity.type].spriteOrigin.y, data[entity.type].sprite, entity.frame);
#ifdef DEBUG_HITBOX
        ab.fillRect(entity.pos.x - data[entity.type].hitbox.x - Game::cameraX, entity.pos.y - data[entity.type].hitbox.y, data[entity.type].hitbox.width, data[entity.type].hitbox.height, WHITE);
#endif
      }
      else
      {
        sprites.drawPlusMask(entity.pos.x - DIE_ANIM_ORIGIN_X - Game::cameraX, entity.pos.y - DIE_ANIM_ORIGIN_Y, fx_destroy_plus_mask, entity.frame);
      }
    }
  }
}
