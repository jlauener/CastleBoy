#include "entity.h"

#include "game.h"
#include "map.h"
#include "player.h"
#include "assets.h"

// TODO handling of hurt, can be handled by entity update so we can delegate specific code instead of hacking in damage function
// FIXME everyXFrames is not precise enough, maybe each entity should have it's own frame counter? --> at least for bone projectile
//  +--> sometime not precise for walk anim too.. visible when boss is hurt..

//#define HURT_INVINCIBLE_THRESHOLD 4
#define DIE_ANIM_ORIGIN_X 4
#define DIE_ANIM_ORIGIN_Y 10

// entity types

// 0000 platform: falling
// 0001 platform: moving
#define ENTITY_FALLING_PLATFORM 0x00
#define ENTITY_MOVING_PLATFORM 0x01
#define PLATFORM_MASK 0xFE

// 0010 candle: coin
// 0011 candle: knife
#define ENTITY_CANDLE_COIN 0x02
#define ENTITY_CANDLE_KNIFE 0x03

// 0100 skeleton: simple
// 0101 skeleton: throw
// 0110 skeleton: armored
// 0111 skeleton: armored + throw
//   ||
//   |+-- throw flag
//   +--- armored flag
#define ENTITY_SKELETON_SIMPLE 0x04
#define ENTITY_SKELETON_THROW 0x05
#define ENTITY_SKELETON_ARMORED 0x06
#define ENTITY_SKELETON_THROW_ARMORED 0x07
#define SKELETON_THROW_FLAG 0x01

// 1000 flyer: skull
#define ENTITY_FLYER_SKULL 0x08

// 1001 bird
#define ENTITY_BIRD 0x09

// 1010 hurler
#define ENTITY_HURLER 0x0A

// 1011 fireball vert
#define ENTITY_FIREBALL_VERT 0x0B

// 1100 candlestick
#define ENTITY_CANDLESTICK 0x0C

// 1101 boss knight
#define ENTITY_BOSS_KNIGHT 0x0D

// 1110 boss harpy
#define ENTITY_BOSS_HARPY 0x0E

// 1111 boss final
#define ENTITY_BOSS_FINAL 0x0F

// pickups
// 10000 pickup: coin
// 10001 pickup: knife
#define ENTITY_PICKUP_COIN 0x10
#define ENTITY_PICKUP_KNIFE 0x11

// projectiles
// 10010 projectile: bone
#define ENTITY_BONE 0x12
// 10011 projectile: fireball horiz
#define ENTITY_FIREBALL_HORIZ 0x13

// state flags
#define FLAG_PRESENT 0x80
#define FLAG_ALIVE 0x40
#define FLAG_MISC1 0x20
#define FLAG_MISC2 0x10
#define MASK_HURT 0x0F
#define HURT_DURATION 0x09

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
  // 0000 platform: falling
  {
    4, 8, // hitbox x, y
    16, 8, // hitbox width, height
    4, 8, // sprite origin x, y
    0, // hp
    entity_falling_platform_plus_mask // sprite
  },
  // 0001 platform: moving
  {
    4, 8, // hitbox x, y
    24, 8, // hitbox width, height
    4, 8, // sprite origin x, y
    0, // hp
    entity_moving_platform_plus_mask // sprite
  },
  // 0010 candle: coin
  {
    2, 8, // hitbox x, y
    4, 6, // hitbox width, height
    2, 10, // sprite origin x, y
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
    3, 16, // hitbox x, y
    6, 16, // hitbox width, height
    6, 16, // sprite origin x, y
    2, // hp
    entity_skeleton_plus_mask // sprite
  },
  // 0101 skeleton: throw
  {
    3, 16, // hitbox x, y
    6, 16, // hitbox width, height
    6, 16, // sprite origin x, y
    2, // hp
    entity_skeleton_plus_mask // sprite
  },
  // 0100 skeleton: armored
  {
    3, 16, // hitbox x, y
    6, 16, // hitbox width, height
    6, 16, // sprite origin x, y
    6, // hp
    entity_skeleton_armored_plus_mask // sprite
  },
  // 0111 skeleton: throw armored (TO BE REMOVED?)
  {
    3, 16, // hitbox x, y
    6, 16, // hitbox width, height
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
  // 1001 bird
  {
    4, 8, // hitbox x, y
    8, 8, // hitbox width, height
    4, 8, // sprite origin x, y
    2, // hp
    entity_bird_plus_mask // sprite
  },
  // 1010 hurler
  {
    4, 8, // hitbox x, y
    8, 8, // hitbox width, height
    4, 8, // sprite origin x, y
    4, // hp
    entity_hurler_plus_mask // sprite
  },
  // 1011 fireball vert
  {
    2, 2, // hitbox x, y
    4, 4, // hitbox width, height
    3, 3, // sprite origin x, y
    0, // hp
    entity_fireball_vert_plus_mask // sprite
  },
  // 1100 candlestick
  {
    5, 4, // hitbox x, y
    10, 4, // hitbox width, height
    6, 8, // sprite origin x, y
    0, // hp
    entity_candlestick_plus_mask // sprite
  },
  // 1101 boss knight
  {
    7, 26, // hitbox x, y
    14, 26, // hitbox width, height
    12, 32, // sprite origin x, y
    BOSS_MAX_HP, // hp
    entity_boss_knight_plus_mask // sprite
  },
  // 1110 boss harpy
  {
    4, 8, // hitbox x, y
    8, 8, // hitbox width, height
    6, 8, // sprite origin x, y
    BOSS_MAX_HP, // hp
    entity_boss_harpy_plus_mask // sprite
  },
  // 1111 boss final
  {
    7, 26, // hitbox x, y
    14, 26, // hitbox width, height
    12, 32, // sprite origin x, y
    BOSS_MAX_HP, // hp
    entity_boss_knight_plus_mask // sprite
  },
  // 10000 pickup: coin
  {
    3, 6, // hitbox x, y
    6, 6, // hitbox width, height
    4, 8, // sprite origin x, y
    0, // hp
    entity_coin_plus_mask // sprite
  },
  // 10001 pickup: knife
  {
    4, 6, // hitbox x, y
    8, 6, // hitbox width, height
    3, 6, // sprite origin x, y
    0, // hp
    entity_knife_plus_mask // sprite
  },
  // 10010 projectile: bone
  {
    3, 3, // hitbox x, y
    6, 6, // hitbox width, height
    4, 4, // sprite origin x, y
    0, // hp
    entity_bone_plus_mask // sprite
  },
  // 10011 projectile: fireball horiz
  {
    2, 2, // hitbox x, y
    4, 4, // hitbox width, height
    3, 3, // sprite origin x, y
    0, // hp
    entity_fireball_horiz_plus_mask // sprite
  }
};

Entity entities[ENTITY_MAX];

uint8_t bossState;
uint8_t bossCounter;
uint8_t bossPhase;

} // unamed

void Entities::init()
{
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    entities[i].state = 0;
  }

  bossState = 0;
  bossCounter = 0;
  bossPhase = 1;
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
      entity.frame = entity.hp > 0 ? 1 : 0; // damageable entities have an hurt frame
      entity.counter = 0;
      return &entity;
    }
  }

  // FIXME assert?
  return NULL;
}

void updateMovingPlatform(Entity& entity)
{
  if (ab.everyXFrames(3))
  {
    entity.pos.x += entity.state & FLAG_MISC1 ? -1 : 1;
    if (++entity.counter == 23)
    {
      entity.counter = 0;
      if (entity.state & FLAG_MISC1)
      {
        entity.state &= ~FLAG_MISC1;
      }
      else
      {
        entity.state |= FLAG_MISC1;
      }
    }
  }
}

void updateSkeleton(Entity& entity)
{
  //  if (entity.state & MASK_HURT)
  //  {
  //    entity.frame = 5;
  //    return;
  //  }

  if (ab.everyXFrames(3))
  {
    if (entity.state & FLAG_MISC2)
    {
      if (++entity.counter == 10)
      {
        Entities::add(ENTITY_BONE, entity.pos.x, entity.pos.y - 10);
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
          if (entity.type & SKELETON_THROW_FLAG && entity.pos.x - Player::pos.x < 94)
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
    // throwing
    entity.frame = 3;
  }
  else if (ab.everyXFrames(8))
  {
    // normal
    entity.frame = entity.frame == 2 ? 1 : 2;
  }
}

void updateHurler(Entity& entity)
{
  //  if (entity.state & MASK_HURT)
  //  {
  //    entity.frame = 5;
  //    return;
  //  }

  if (ab.everyXFrames(3))
  {
    if (entity.counter < 30) entity.counter++;
    if (entity.counter == 30 && entity.pos.x - Player::pos.x < 94)
    {
      Entities::add(ENTITY_FIREBALL_HORIZ, entity.pos.x, entity.pos.y - 4);
      entity.counter = 0;
    }
  }

  entity.frame = entity.counter < 20 ? 1 : 2;
}

void updateFlyer(Entity& entity)
{
  //  if (entity.state & MASK_HURT)
  //  {
  //    // hurt
  //    return;
  //  }

  if (!(entity.state & FLAG_MISC1) && entity.pos.x - Player::pos.x < 90)
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
      entity.frame = entity.frame == 2 ? 1 : 2;
    }
  }
}

void updateBird(Entity& entity)
{
  // FLAG_MISC1 is used for direction (0 going left, 1 going right)

  if (ab.everyXFrames(2))
  {
    entity.pos.x += entity.state & FLAG_MISC1 ? 1 : -1;
    if (++entity.counter == 104)
    {
      entity.state = entity.state & FLAG_MISC1 ? entity.state & ~FLAG_MISC1 : entity.state | FLAG_MISC1;
      entity.counter = 0;
    }

    // attacking
    if (entity.counter % 4)
    {
      entity.pos.y += entity.counter < 52 ? 1 : -1;
    }
  }

  if (ab.everyXFrames(ENTITY_BIRD_WALK_INTERVAL))
  {
    if (entity.state & FLAG_MISC1)
    {
      entity.frame = entity.frame == 5 ? 4 : 5;
    }
    else
    {
      entity.frame = entity.frame == 2 ? 1 : 2;
    }
  }
}

void updateBossKnight(Entity& entity)
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

  if (ab.everyXFrames(BOSS_KNIGHT_WALK_INTERVAL))
  {
    if (entity.state & FLAG_MISC1)
    {
      entity.frame = entity.frame == 6 ? 5 : 6;
    }
    else
    {
      entity.frame = entity.frame == 2 ? 1 : 2;
    }
  }
}

void updateBossHarpy(Entity& entity)
{
  // FLAG_MISC1 is use for direction (0 going left, 1 going right)
  // FLAG_MISC2 is use to make harpy invulnerable after being it

  if (ab.everyXFrames(4 - bossPhase))
  {
    entity.pos.x += entity.state & FLAG_MISC1 ? 1 : -1;
    if (++entity.counter == 104)
    {
      entity.state = entity.state & FLAG_MISC1 ? entity.state & ~FLAG_MISC1 : entity.state | FLAG_MISC1;
      entity.counter = 0;

      if (++bossState == 3)
      {
        if (entity.state & FLAG_MISC2)
        {
          // got hurt
          if (entity.hp == 8)
          {
            bossPhase = 2;
          }
          else if (entity.hp == 4)
          {
            bossPhase = 3;
          }
          entity.state &= ~FLAG_MISC2;
        }
        bossState = 0;
      }
    }

    if (bossState < 2)
    {
      // flying
      if (++bossCounter >= 12 + bossPhase * 3)
      {
        Entities::add(ENTITY_FIREBALL_VERT, entity.pos.x, entity.pos.y);
        bossCounter = 0;
      }
    }
    else
    {
      // attacking
      if (entity.counter % 4)
      {
        entity.pos.y += entity.counter < 52 ? 1 : -1;
      }
    }
  }

  if (bossState < 2 || entity.counter >= 52)
  {
    // flying
    if (ab.everyXFrames(BOSS_HARPY_WALK_INTERVAL / bossPhase))
    {
      if (entity.state & FLAG_MISC1)
      {
        entity.frame = entity.frame == 6 ? 5 : 6;
      }
      else
      {
        entity.frame = entity.frame == 2 ? 1 : 2;
      }
    }
  }
  else
  {
    // going down
    entity.frame = entity.state & FLAG_MISC1 ? 7 : 3;
  }
}

void updateBossFinal(Entity& entity)
{
  if (entity.hp == 8)
  {
    bossPhase = 2;
  }
  else if (entity.hp == 4)
  {
    bossPhase = 3;
  }

  if (ab.everyXFrames(4 - bossPhase))
  {
    if (++entity.counter == 40)
    {
      Entities::add(ENTITY_FIREBALL_HORIZ, entity.pos.x, entity.pos.y - 4 - bossState * 8);
      entity.counter = 0;
      ++bossState %= 2;
    }
  }

  entity.frame = 1; // TODO
}

void updateProjectile(Entity& entity)
{
  --entity.pos.x;
  if (entity.type == ENTITY_BONE)
  {
    entity.pos.y += entity.counter - 2;
    if (entity.counter < 8 && ab.everyXFrames(10))
    {
      ++entity.counter;
    }
  }

  if (entity.pos.y > 68 || entity.pos.x < Game::cameraX - 8)
  {
    entity.state = 0;
  }
  if (ab.everyXFrames(12))
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
      if (entity.state & MASK_HURT)
      {
        uint8_t hurtCounter = entity.state & MASK_HURT;
        entity.state &= ~MASK_HURT;
        entity.state |= --hurtCounter;
        if (hurtCounter == 0)
        {
          if (entity.hp == 0)
          {
            entity.frame = 0;
            entity.counter = 0;
          }
          else
          {
            if (entity.type == ENTITY_BOSS_KNIGHT)
            {
              // FIXME maybe this is not needed with proper boss update?
              entity.frame = entity.state & FLAG_MISC1 ? 5 : 1;
            }
            else if (entity.type == ENTITY_BOSS_HARPY)
            {
              // FIXME maybe this is not needed with proper boss update?
              entity.frame = entity.state & FLAG_MISC1 ? 7 : 3;
            }
            else
            {
              entity.frame = 1;
            }
          }
        }
      }
      else if (entity.state & FLAG_ALIVE)
      {
        switch (entity.type)
        {
          case ENTITY_FALLING_PLATFORM:
            if (entity.state & FLAG_MISC1)
            {
              if (++entity.counter == ENTITY_FALLING_PLATFORM_DURATION)
              {
                entity.state = 0;
              }
              else if (entity.counter == ENTITY_FALLING_PLATFORM_WARNING)
              {
                entity.frame = 1;
              }
            }
            break;
          case ENTITY_MOVING_PLATFORM:
            updateMovingPlatform(entity);
            break;
          case ENTITY_FIREBALL_VERT:
            if (++entity.pos.y == 68)
            {
              if (Map::boss != NULL)
              {
                // special case: harpy boss throws fireball that don't 'cycle'
                entity.state = 0;
              }
              else
              {
                entity.pos.y = -4;
              }
            }
            if (ab.everyXFrames(12))
            {
              ++entity.frame %= 2;
            }
            break;
          case ENTITY_CANDLE_COIN:
          case ENTITY_CANDLE_KNIFE:
            if (ab.everyXFrames(8))
            {
              ++entity.frame %= 2;
            }
            break;
          case ENTITY_PICKUP_COIN:
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
          case ENTITY_BIRD:
            updateBird(entity);
            break;
          case ENTITY_HURLER:
            updateHurler(entity);
            break;
          case ENTITY_CANDLESTICK:
            if (entity.pos.x < Player::pos.x + 4)
            {
              entity.state |= FLAG_MISC1;
            }

            if (entity.state & FLAG_MISC1 && Game::moveY(entity.pos, 1, data[entity.type].hitbox))
            {
              entity.state &= ~FLAG_ALIVE;
            }
            break;
          case ENTITY_BOSS_KNIGHT:
            updateBossKnight(entity);
            break;
          case ENTITY_BOSS_HARPY:
            updateBossHarpy(entity);
            break;
          case ENTITY_BOSS_FINAL:
            updateBossFinal(entity);
            break;
          case ENTITY_BONE:
          case ENTITY_FIREBALL_HORIZ:
            updateProjectile(entity);
            break;
        }
      }
      else
      {
        if (++entity.counter == 8)
        {
          if (++entity.frame == 3)
          {
            if (entity.type == ENTITY_CANDLE_COIN)
            {
              // special case: CANDLE_COIN spawns a COIN pickup
              entity.type = ENTITY_PICKUP_COIN;
              entity.state |= FLAG_ALIVE;
              entity.frame = 0;
            }
            else if (entity.type == ENTITY_CANDLE_KNIFE)
            {
              // special case: CANDLE_KNIFE spawns a KNIFE pickup
              entity.type = ENTITY_PICKUP_KNIFE;
              entity.state |= FLAG_ALIVE;
              entity.frame = 0;
            }
            else
            {
              entity.state = 0;
            }
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

        bool damage = true;
        if (entity.type == ENTITY_BOSS_KNIGHT)
        {
          // special case: boss1 can only be hit from the back
          damage = entity.state & FLAG_MISC1 ? Player::pos.x < entity.pos.x : Player::pos.x > entity.pos.x;
          if (damage)
          {
            entity.state |= FLAG_MISC2; // use flag MISC2 to tell the boss he has been hurt and should revert
            entity.frame = 0;
          }
          else
          {
            entity.frame = 3;
            entity.state |= MASK_HURT; // when boss resist, stop moving a bit longer than when it's a normal hurt
            sound.tone(NOTE_GS2, 15);
          }

          if (entity.state & FLAG_MISC1)
          {
            entity.frame += 4;
          }
          entity.state |= HURT_DURATION;
        }
        else if (entity.type == ENTITY_BOSS_HARPY)
        {
          // special case: harpy
          damage = !(entity.state & FLAG_MISC2);
          if (damage)
          {
            entity.frame = entity.state & FLAG_MISC1 ? 4 : 0;
            entity.state |= FLAG_MISC2;
            entity.state |= HURT_DURATION;
          }
        }
        // TODO ENTITY_BOSS_FINAL
        else
        {
          entity.frame = 0;
          entity.state |= HURT_DURATION;
        }

        if (damage)
        {
          if (entity.hp <= value)
          {
            if (entity.type == ENTITY_CANDLE_COIN || entity.type == ENTITY_CANDLE_KNIFE)
            {
              // candles don't have hurt anim and have faster die anim
              entity.state &= ~MASK_HURT;
              entity.frame = 1;
            }
            entity.state &= ~FLAG_ALIVE;
            entity.hp = 0;
            entity.counter = 0;
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
  }

  return hit;
}

bool Entities::moveCollide(int16_t x, int8_t y, const Box& hitbox)
{
  bool collide = false;
  for (uint8_t i = 0; i < ENTITY_MAX; i++)
  {
    Entity& entity = entities[i];
    if (entity.state & FLAG_ALIVE && !(entity.type & PLATFORM_MASK))
    {
      const Box& entityHitbox = data[entity.type].hitbox;
      if (Util::collideRect(entity.pos.x - entityHitbox.x,
                            entity.pos.y - entityHitbox.y,
                            entityHitbox.width,
                            entityHitbox.height,
                            x - hitbox.x, y - hitbox.y, hitbox.width, hitbox.height))
      {
        collide = true;
        if (entity.type == ENTITY_FALLING_PLATFORM)
        {
          entity.state |= FLAG_MISC1;
        }
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
    if (entity.state & FLAG_ALIVE && !(entity.state & MASK_HURT) && entity.type != ENTITY_CANDLE_COIN && entity.type != ENTITY_CANDLE_KNIFE)
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
          case ENTITY_FALLING_PLATFORM:
          case ENTITY_MOVING_PLATFORM:
            // platforms do nothing
            break;
          case ENTITY_PICKUP_COIN:
            //Game::timeLeft += PICKUP_COIN_VALUE;
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
      if (entity.state & FLAG_ALIVE || entity.state & MASK_HURT)
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
