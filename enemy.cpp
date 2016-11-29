#include "enemy.h"

#include "assets.h"

namespace
{

struct EnemyData
{
  Rect hitbox;
  Point spriteOrigin;
  const uint8_t* sprite;
};

const EnemyData data[] =
{
  // skeleton
  {
    3, 14, // hitbox x, y
    6, 14, // hitbox width, height
    8, 16, // sprite origin x, y
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
  int8_t dir;
  uint8_t moveDist;
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
      enemies[i].dir = -1;
      enemies[i].moveDist = 0;
      return;
    }
  }

  // FIXME assert?
}

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

void updateSkeleton(Enemy& enemy)
{
  if(ab.everyXFrames(4))
  {
    enemy.x += enemy.dir;
    if(++enemy.moveDist == 23)
    {
      enemy.moveDist = 0;
      enemy.dir = enemy.dir == -1 ? 1 : -1;
    }
  }
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
      if (ab.everyXFrames(6))
      {
        ++enemies[i].frame %= 2;
      }
      sprites.drawPlusMask(enemies[i].x - data[enemies[i].type].spriteOrigin.x - cameraX, enemies[i].y - data[enemies[i].type].spriteOrigin.y, data[enemies[i].type].sprite, enemies[i].frame);

#ifdef DEBUG_HITBOX
     ab.fillRect(enemies[i].x - data[enemies[i].type].hitbox.x - cameraX, enemies[i].y - data[enemies[i].type].hitbox.y, data[enemies[i].type].hitbox.width, data[enemies[i].type].hitbox.height);
#endif
    }
  }
}

