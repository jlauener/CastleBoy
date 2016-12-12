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
  int8_t spriteOriginX;
  int8_t spriteOriginY;
  uint8_t hp;
  const uint8_t* sprite;
};

// TODO use PROGMEM?
const EntityData data[] =
{
  // 0000 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 0001 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 0010 candle: coin
  {
    2, 8, // hitbox x, y
    4, 6, // hitbox width, height
    4, 10, // sprite origin x, y
    1, // hpm
    entity_candle_plus_mask // sprite
  },
  // 0011 candle: powerup
  {
    2, 8, // hitbox x, y
    4, 6, // hitbox width, height
    4, 10, // sprite origin x, y
    1, // hpm
    entity_candle_plus_mask // sprite
  },
  // 0100 skeleton: simple
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
    2, // hp
    entity_skeleton_plus_mask // sprite
  },
  // 0101 skeleton: throw
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
    2, // hp
    entity_skeleton_plus_mask // sprite
  },
  // 0100 skeleton: armored
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
    6, // hp
    entity_skeleton_plus_mask // sprite
  },
  // 0111 skeleton: throw armored
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
    6, // hp
    entity_skeleton_plus_mask // sprite
  },
  // 1000 flyer: skull
  {
    2, 6, // hitbox x, y
    4, 6, // hitbox width, height
    4, 8, // sprite origin x, y
    1, // hp
    entity_skull_plus_mask // sprite
  },
  // 1001 flyer: ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 1010 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 1011 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 1100 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 1101 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 1110 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 1111 ???
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 10000 pickup: coin
  {
    3, 6, // hitbox x, y
    6, 6, // hitbox width, height
    4, 8, // sprite origin x, y
    0, // hp
    entity_coin_plus_mask // sprite
  },
  // 10001 pickup: heart
  {
    3, 6, // hitbox x, y
    6, 6, // hitbox width, height
    4, 8, // sprite origin x, y
    0, // hp
    entity_heart_plus_mask // sprite
  },
  // 10010 pickup: knife
  {
    4, 6, // hitbox x, y
    8, 6, // hitbox width, height
    3, 6, // sprite origin x, y
    0, // hp
    entity_knife_plus_mask // sprite
  }
};

Entity entities[ENTITY_MAX];
} // unamed

void Entities::init()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    entities[i].state = 0;
  }
}

void Entities::add(uint8_t type, uint8_t tileX, uint8_t tileY)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state == 0)
    {
      entity.type = type;
      entity.pos.x = tileX * TILE_WIDTH + HALF_TILE_WIDTH;
      entity.pos.y = tileY * TILE_HEIGHT + TILE_HEIGHT;
      entity.hp = data[type].hp;
      entity.state = FLAG_ACTIVE | FLAG_ALIVE;
//      entity.active = true;
//      entity.alive = true;
//      entity.flag = false;
      entity.frame = 0;
      entity.counter = 0;
      return;
    }
  }

  // FIXME assert?
}

inline void updateSkeleton(Entity& entity)
{
  if (ab.everyXFrames(3))
  {
    entity.pos.x += entity.state & FLAG_MISC ? 1 : -1;
    if (++entity.counter == 23)
    {
      entity.counter = 0;
      if(entity.state & FLAG_MISC)
      {
        entity.state &= ~FLAG_MISC;
      }
      else
      {
        entity.state |= FLAG_MISC;
      }
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
  if (!(entity.state & FLAG_MISC) && entity.pos.x - Player::pos.x < 72)
  {
    entity.state |= FLAG_MISC;
  }

  if (entity.state & FLAG_MISC)
  {
    if (ab.everyXFrames(2))
    {
      --entity.pos.x;
      if (entity.pos.x < -8)
      {
        entity.state &= ~FLAG_ACTIVE;
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
    if (entity.state & FLAG_ACTIVE)
    {
      if (entity.state & FLAG_ALIVE)
      {
        switch (entity.type)
        {
          case ENTITY_CANDLE_COIN:
          case ENTITY_CANDLE_POWERUP:
            if (ab.everyXFrames(8))
            {
              ++entity.frame %= 2;
            }
            break;
          case ENTITY_PICKUP_COIN:
          case ENTITY_PICKUP_HEART:
          case ENTITY_PICKUP_KNIFE:
            Map::moveY(entity.pos, 2, data[entity.type].hitbox);
            if (entity.type != ENTITY_PICKUP_KNIFE && ab.everyXFrames(12))
            {
              ++entity.frame %= 2;
            }
            break;
          case ENTITY_SKELETON_SIMPLE:
          case ENTITY_SKELETON_THROW:
          case ENTITY_SKELETON_ARMORED:
          case ENTITY_SKELETON_THROW_ARMORED:
            updateSkeleton(entity);
            break;
          case ENTITY_FLYER_SKULL:
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
            entity.state = 0;
          }
          entity.counter = 0;
        }
      }
    }
  }
}

bool Entities::damage(int16_t x, int8_t y, uint8_t width, uint8_t height, uint8_t value)
{
  bool hit = false;
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state & FLAG_ALIVE)
    {
      const EntityData& entityData = data[entity.type];
      if (entityData.hp > 0 &&
          Util::collideRect(entity.pos.x - entityData.hitbox.x, entity.pos.y - entityData.hitbox.y, entityData.hitbox.width, entityData.hitbox.height, x, y, width, height))
      {
        hit = true;
        if (entity.hp <= value)
        {         
          if (entity.type == ENTITY_CANDLE_COIN)
          {
            // special case: CANDLE_COIN spawns a COIN
            entity.type = ENTITY_PICKUP_COIN;
          }
          else if(entity.type == ENTITY_CANDLE_POWERUP)
          {
            // special case: CANDLE_POWERUP spawns an HEART or KNIFE            
            entity.type = Player::hp == PLAYER_MAX_HP ? ENTITY_PICKUP_KNIFE : ENTITY_PICKUP_HEART;
          }
          else
          {
            entity.state &= ~FLAG_ALIVE;            
          }
          entity.counter = 0;
          entity.frame = 0;
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

  if (hit)
  {
    LOG_DEBUG(1);
  }
  return hit;
}

Entity* Entities::collide(int16_t x, int8_t y, uint8_t width, uint8_t height)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state & FLAG_ALIVE && entity.type != ENTITY_CANDLE_COIN && entity.type != ENTITY_CANDLE_POWERUP)
    {
      const Box& entityHitbox = data[entity.type].hitbox;
      if (Util::collideRect(entity.pos.x - entityHitbox.x, entity.pos.y - entityHitbox.y, entityHitbox.width, entityHitbox.height, x, y, width, height))
      {        
        if (data[entity.type].hp == 0)
        {
          // special case: pickups don't have HP and don't damage player
          entity.state = 0;
          sound.tone(NOTE_CS6, 30, NOTE_CS5, 40);

          switch(entity.type)
          {
            case ENTITY_PICKUP_COIN:
              // TODO
              // Menu::timeLeft += PICKUP_COIN_VALUE;
              break;
            case ENTITY_PICKUP_HEART:              
              if(Player::hp < PLAYER_MAX_HP)
              {
                ++Player::hp;
              }
              break;
            case ENTITY_PICKUP_KNIFE:
              // TODO
              Player::knifeCount += PICKUP_KNIFE_VALUE;
              break;
          }
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
    if (entity.state & FLAG_ACTIVE)
    {
      if (entity.state & FLAG_ALIVE)
      {
        sprites.drawPlusMask(entity.pos.x - data[entity.type].spriteOriginX - Game::cameraX, entity.pos.y - data[entity.type].spriteOriginY, data[entity.type].sprite, entity.frame);
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
