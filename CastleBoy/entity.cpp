#include "entity.h"

#include "game.h"
#include "map.h"
#include "player.h"
#include "assets.h"

// TODO refactor damage and collide (let's wait for falling blocks)

#define DIE_ANIM_ORIGIN_X 4
#define DIE_ANIM_ORIGIN_Y 10

// entity types

// 0000 falling tile
#define ENTITY_FALLING_TILE 0x00

// 0001 ???

// 0010 candle: coin
// 0011 candle: powerup
//   ||
//   |+-- coin/powerup flag
//   +--- candle flag
#define ENTITY_CANDLE_COIN 0x02
#define ENTITY_CANDLE_POWERUP 0x03

// 0100 skeleton: simple
// 0101 skeleton: throw
// 0110 skeleton: armored
// 0111 skeleton: armored + throw
//  |||
//  ||+-- throw flag
//  |+--- armored flag
//  +---- skeleton flag
#define ENTITY_SKELETON_SIMPLE 0x04
#define ENTITY_SKELETON_THROW 0x05
#define ENTITY_SKELETON_ARMORED 0x06
#define ENTITY_SKELETON_THROW_ARMORED 0x07
#define SKELETON_THROW_FLAG 0x01

// 1000 flyer: skull
#define ENTITY_FLYER_SKULL 0x08

// 1001 flyer: ??? 0x09
// 1010 ??? 0x0A
// 1011 ??? 0x0B
// 1100 ??? 0x0C
// 1101 boss 1
#define ENTITY_BOSS_1 0x0D
// 1110 boss 2 0x0E
// 1111 boss 3 0x0F

// pickups
// 10000 pickup: coin
// 10001 pickup: heart
// 10010 pickup: knife
#define ENTITY_PICKUP_COIN 0x10
#define ENTITY_PICKUP_HEART 0x11
#define ENTITY_PICKUP_KNIFE 0x12

// projectiles
// 10011 projectile: bone
#define ENTITY_PROJECTILE_BONE 0x13

// state flags
#define FLAG_PRESENT 0x80
#define FLAG_ALIVE 0x40
#define FLAG_MISC1 0x20
#define FLAG_MISC2 0x10
#define MASK_HURT 0x0F

namespace
{
// entity data
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
  // 0000 falling tile
  {
    4, 8, // hitbox x, y
    16, 8, // hitbox width, height
    4, 8, // sprite origin x, y
    0, // hp
    entity_falling_tile_plus_mask // sprite
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
    1, // hp
    entity_candle_plus_mask // sprite
  },
  // 0011 candle: powerup
  {
    2, 8, // hitbox x, y
    4, 6, // hitbox width, height
    4, 10, // sprite origin x, y
    1, // hp
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
  // 1001 reserved flyer 2
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
  // 1101 boss 1
  {
    8, 26, // hitbox x, y
    16, 26, // hitbox width, height
    12, 32, // sprite origin x, y
    BOSS_MAX_HP, // hp
    entity_boss1_plus_mask // sprite
  },
  // 1110 reserved boss 2
  {
    0, 0, // hitbox x, y
    0, 0, // hitbox width, height
    0, 0, // sprite origin x, y
    0, // hp
    NULL // sprite
  },
  // 1111 reserved boss 3
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
  },
  // 10011 projectile: bone
  {
    3, 3, // hitbox x, y
    6, 6, // hitbox width, height
    4, 4, // sprite origin x, y
    0, // hp
    entity_bone_plus_mask // sprite
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

Entity* Entities::add(uint8_t type, int16_t x, int8_t y)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state == 0)
    {
      entity.type = type;
      entity.pos.x = x;
      entity.pos.y = y;
      entity.hp = data[type].hp;
      entity.state = FLAG_PRESENT | FLAG_ALIVE;
      entity.frame = 0;
      entity.counter = 0;
      return &entity;
    }
  }

  // FIXME assert?
  return NULL;
}

void updateSkeleton(Entity& entity)
{
  if (ab.everyXFrames(3))
  {
    if (entity.state & FLAG_MISC2)
    {
      if (++entity.counter == 10)
      {
        Entities::add(ENTITY_PROJECTILE_BONE, entity.pos.x, entity.pos.y - 10);
        entity.state &= ~FLAG_MISC2;
        entity.counter = 0;
      }
    }
    else
    {
      entity.pos.x += entity.state & FLAG_MISC1 ? 1 : -1;
      if (++entity.counter == 23)
      {
        entity.counter = 0;
        if (entity.state & FLAG_MISC1)
        {
          entity.state &= ~FLAG_MISC1;
        }
        else
        {
          if (entity.type & SKELETON_THROW_FLAG && entity.pos.x - Player::pos.x < 72)
          {
            // start throwing bone
            entity.state |= FLAG_MISC2;
          }
          entity.state |= FLAG_MISC1;
        }
      }
    }
  }

  if (entity.state & FLAG_MISC2)
  {
    entity.frame = 4;
  }
  else if (ab.everyXFrames(8))
  {
    ++entity.frame %= 2;
    if (entity.hp > 2)
    {
      // use armored frame
      entity.frame += 2;
    }
  }
}

void updateBoss1(Entity& entity)
{
  if (ab.everyXFrames(4))
  {
    if (entity.state & FLAG_MISC2)
    {
      // boss got hurt, change direction
      entity.state &= ~FLAG_MISC2;
      Util::toggle(entity.state, FLAG_MISC1);
      entity.counter = 87 - entity.counter;
    }

    entity.pos.x += entity.state & FLAG_MISC1 ? 1 : -1;
    if (++entity.counter == 87)
    {
      entity.counter = 0;
      Util::toggle(entity.state, FLAG_MISC1);
    }
  }

  if (ab.everyXFrames(10))
  {
    ++entity.frame %= 2;
    if (entity.state & FLAG_MISC1)
    {
      entity.frame += 2;
    }
  }
}

void updateFlyer(Entity& entity)
{
  if (!(entity.state & FLAG_MISC1) && entity.pos.x - Player::pos.x < 72)
  {
    entity.state |= FLAG_MISC1;
  }

  if (entity.state & FLAG_MISC1)
  {
    if (ab.everyXFrames(2))
    {
      --entity.pos.x;
      if (entity.pos.x < -8)
      {
        entity.state  = 0;
      }

      entity.pos.y += ++entity.counter / 20 % 2 ? 1 : -1;
    }
    if (ab.everyXFrames(8))
    {
      ++entity.frame %= 2;
    }
  }
}

void updateProjectileBone(Entity& entity)
{
  --entity.pos.x;
  entity.pos.y += entity.counter - 2;
  if (entity.counter < 8 && ab.everyXFrames(10))
  {
    ++entity.counter;
  }
  if (entity.pos.y > 68)
  {
    entity.state = 0;
  }
  if (ab.everyXFrames(8))
  {
    ++entity.frame %= 2;
  }
}

void Entities::update()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state & FLAG_PRESENT)
    {
      if (entity.state & FLAG_ALIVE)
      {
        if (entity.state & MASK_HURT)
        {
          if (ab.everyXFrames(2))
          {
            uint8_t hurtCounter = entity.state & MASK_HURT;
            entity.state &= ~MASK_HURT;
            entity.state |= --hurtCounter;
          }
        }

        if (!(entity.state & MASK_HURT))
        {
          switch (entity.type)
          {
            case ENTITY_FALLING_TILE:
              if (entity.state & FLAG_MISC1)
              {
                if (++entity.counter == 30)
                {
                  entity.state = 0;
                }
              }
              break;
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
              Game::moveY(entity.pos, 2, data[entity.type].hitbox);
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
              updateFlyer(entity);
              break;
            case ENTITY_BOSS_1:
              updateBoss1(entity);
              break;
            case ENTITY_PROJECTILE_BONE:
              updateProjectileBone(entity);
              break;
          }
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
    if (entity.state & FLAG_ALIVE && !(entity.state & MASK_HURT))
    {
      const EntityData& entityData = data[entity.type];
      if (entityData.hp > 0 && // entity with no HP cannot be damaged
          Util::collideRect(entity.pos.x - entityData.hitbox.x,
                            entity.pos.y - entityData.hitbox.y,
                            entityData.hitbox.width,
                            entityData.hitbox.height,
                            x, y, width, height))
      {
        hit = true;

        bool hurt = true;
        if (entity.type == ENTITY_BOSS_1)
        {
          // special case: boss1 can only be hit from the back
          if (entity.state & FLAG_MISC1)
          {
            hurt = Player::pos.x < entity.pos.x;
          }
          else
          {
            hurt = Player::pos.x > entity.pos.x;
          }
          if (hurt)
          {
            entity.state |= FLAG_MISC2; // use flag MISC2 to tell the boss he has been hurt and should revert
          }
          else
          {
            sound.tone(NOTE_G2, 15);
          }
        }

        if (hurt)
        {
          if (entity.hp <= value)
          {
            if (entity.type == ENTITY_CANDLE_COIN)
            {
              // special case: CANDLE_COIN spawns a COIN
              entity.type = ENTITY_PICKUP_COIN;
            }
            else if (entity.type == ENTITY_CANDLE_POWERUP)
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
            entity.hp = 0;
            sound.tone(NOTE_CS3H, 30);
          }
          else
          {
            entity.hp -= value;
            entity.state |= MASK_HURT;
            sound.tone(NOTE_CS3H, 15);
          }
        }
      }
    }
  }

  return hit;
}

bool Entities::moveCollide(int16_t x, int8_t y, const Box& hitbox)
{
  bool collide = false;
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state & FLAG_ALIVE && entity.type == ENTITY_FALLING_TILE)
    {
      const Box& entityHitbox = data[entity.type].hitbox;
      if (Util::collideRect(entity.pos.x - entityHitbox.x,
                            entity.pos.y - entityHitbox.y,
                            entityHitbox.width,
                            entityHitbox.height,
                            x - hitbox.x, y - hitbox.y, hitbox.width, hitbox.height))
      {
        collide = true;
        entity.state |= FLAG_MISC1;
        entity.frame = 1;
      }
    }
  }
  return collide;
}

Entity* Entities::checkPlayer(int16_t x, int8_t y, uint8_t width, uint8_t height)
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state & FLAG_ALIVE && entity.type != ENTITY_CANDLE_COIN && entity.type != ENTITY_CANDLE_POWERUP)
    {
      const Box& entityHitbox = data[entity.type].hitbox;
      if (Util::collideRect(entity.pos.x - entityHitbox.x,
                            entity.pos.y - entityHitbox.y,
                            entityHitbox.width,
                            entityHitbox.height,
                            x, y, width, height))
      {
        switch (entity.type)
        {
          case ENTITY_FALLING_TILE:
            // falling tile does nothing
            break;
          case ENTITY_PICKUP_COIN:
            Game::timeLeft += PICKUP_COIN_VALUE;
            entity.state = 0;
            sound.tone(NOTE_CS6, 30, NOTE_CS5, 40);
            break;
          case ENTITY_PICKUP_HEART:
            if (Player::hp < PLAYER_MAX_HP)
            {
              ++Player::hp;
            }
            entity.state = 0;
            sound.tone(NOTE_CS6, 30, NOTE_CS5, 40);
            break;
          case ENTITY_PICKUP_KNIFE:
            Player::knifeCount += PICKUP_KNIFE_VALUE;
            entity.state = 0;
            sound.tone(NOTE_CS6, 30, NOTE_CS5, 40);
            break;
          default:
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
    if (entity.state & FLAG_PRESENT)
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
