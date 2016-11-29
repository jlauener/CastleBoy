#include "enemy.h"

#include "assets.h"

namespace
{
  
struct EnemyData
{
  Rect hitbox;
  const uint8_t* sprite;
};

const EnemyData data[] =
{
  // skeleton
  {
      // hitbox
      {
        4, 14, // x, y
        8, 14  // width, height
      },
      skeleton_plus_mask // sprite
  }
};

struct Enemy
{
  uint8_t type;
  int16_t x;
  int16_t y;
  // TODO use bitmask
  bool active;
  uint8_t frame;
};

Enemy enemies[ENEMY_MAX];

} // unamed

void Enemies::init()
{
  for (byte i = 0; i < ENEMY_MAX; i++)
  {
    enemies[i].active = false;
  }
}

void Enemies::add(uint8_t type, int16_t x, int16_t y)
{
  for (byte i = 0; i < ENEMY_MAX; i++)
  {
    if (!enemies[i].active)
    {
      enemies[i].type = type;
      enemies[i].x = x;
      enemies[i].y = y;
      enemies[i].active = true;
      enemies[i].frame = 0;
      return;
    }
  }

  // FIXME assert?
}

Rect skeletonHitbox;

void Enemies::hit(int16_t x, int16_t y, int16_t w)
{
  for (byte i = 0; i < ENEMY_MAX; i++)
  {
    if (enemies[i].active)
    {
      if (Util::collideHLine(x, y, w, enemies[i].x, enemies[i].y, data[enemies[i].type].hitbox))
      {
        enemies[i].active = false;
      }
    }
  }
}

void updateSkeleton(Enemy& skeleton)
{
  // TODO  
}

void Enemies::update()
{
  for (byte i = 0; i < ENEMY_MAX; i++)
  {
    if (enemies[i].active)
    {
      switch (enemies[i].type)
      {
        case ENEMY_SKELETON:
          updateSkeleton(enemies[i]);
        break;
      }
    }
  }
}

void Enemies::draw()
{
  for (byte i = 0; i < ENEMY_MAX; i++)
  {
    if (enemies[i].active)
    {
      if (ab.everyXFrames(8))
      {
        ++enemies[i].frame %= 2;
      }
      sprites.drawPlusMask(enemies[i].x - cameraX, enemies[i].y - 2, data[enemies[i].type].sprite, enemies[i].frame);
    }
  }
}

