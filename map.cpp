#include "map.h"

#include "assets.h"
#include "entity.h"

// map data
#define TILE_EMPTY 0
#define TILE_WALL 1
#define TILE_GROUND 2

int16_t Map::width;

namespace
{
const uint8_t* tilemap;
int16_t height;

uint8_t getTileAt(int16_t x, int16_t y)
{
  uint8_t mask = 0x03 << (y % 4) * 2;
  return (pgm_read_byte(tilemap + (x * height + y) / 4) & mask) >> (y % 4) * 2;
}

}  // unamed

void Map::init(const uint8_t* source)
{
  width = pgm_read_byte(source);
  height = pgm_read_byte(++source);
  tilemap = ++source;
  cameraX = 0;

  // load entities
  source += width * height / 4;
  uint8_t entityCount = pgm_read_byte(source);
  for (uint8_t i = 0; i < entityCount; i++)
  {
    uint8_t temp = pgm_read_byte(++source);
    uint8_t entityType = (temp & 0xF0) >> 4;
    uint8_t y = temp & 0x0F;
    uint8_t x = pgm_read_byte(++source);
    Entities::add(entityType, x * TILE_WIDTH + HALF_TILE_WIDTH, y * TILE_HEIGHT + TILE_HEIGHT);
  }
}

bool Map::collide(int16_t x, int16_t y, const Rect& hitbox)
{
  x -= hitbox.x;
  y -= hitbox.y;

  if (x < 0 /*|| x + hitbox.width > width * TILE_WIDTH*/)
  {
    // cannot get out on the sides, collide
    // WARNING can get out of right side, if we use this for projectile
    // we might need to change this..
    //LOG_DEBUG("side");
    return true;
  }

  int16_t tx1 = x / TILE_WIDTH;
  int16_t ty1 = y / TILE_HEIGHT;
  int16_t tx2 = (x + hitbox.width - 1) / TILE_WIDTH;
  int16_t ty2 = (y + hitbox.height - 1) / TILE_HEIGHT;

  if (ty2 < 0 || ty2 >= height)
  {
    // either higher or lower than map, no collision
    //LOG_DEBUG("higher or lower");
    return false;
  }

  // clamp positions
  if (tx1 < 0) tx1 = 0;
  if (tx2 >= width) tx2 = width - 1;
  if (ty1 < 0) ty1 = 0;
  if (ty2 >= height) ty2 = height - 1;

  // perform hit test on selected tiles
  for (int16_t ix = tx1; ix <= tx2; ix++)
  {
    for (int16_t iy = ty1; iy <= ty2; iy++)
    {
      // FIXME
      if (getTileAt(ix, iy) > 0)
      {
        // check for rectangle intersection
        if (ix * TILE_WIDTH + TILE_WIDTH > x && iy * TILE_HEIGHT + TILE_HEIGHT > y && ix * TILE_WIDTH < x + hitbox.width && iy * TILE_HEIGHT < y + hitbox.height)
        {
          //LOG_DEBUG("hit");
          return true;
        }
      }
    }
  }

  return false;
}

bool Map::moveY(int16_t x, int16_t& y, int16_t dy, const Rect& hitbox)
{
  if (dy != 0)
  {
    int8_t sign = dy > 0 ? 1 : -1;
    while (dy != 0)
    {
      if (Map::collide(x, y + sign, hitbox))
      {
        return true;
      }
      y += sign;
      dy -= sign;
    }
  }

  return false;
}

void Map::draw()
{
  uint8_t start = cameraX / 8;
  bool isGround;
  bool isBlock;

  // FIXME can be optimized
  for (uint8_t ix = start; ix < start + 17; ix++)
  {
    isGround = false;
    isBlock = false;
    for (uint8_t iy = 0; iy < height; iy++)
    {
      uint8_t tile = getTileAt(ix, iy);
      switch (tile)
      {
        case TILE_EMPTY:
          if (isBlock)
          {
            isBlock = false;
            tile = 4;
          }
          else
          {
            tile = 0;
          }

          isGround = false;
          break;
        case TILE_WALL:
          tile = 1;
          isBlock = true;
          isGround = false;
          break;
        case TILE_GROUND:
          if (isGround)
          {
            // already in ground, use inner ground tile
            tile = 2;
          }
          else
          {
            // first ground tile
            tile = 3;
            isGround = true;
          }
          isBlock = false;
          break;
      }
      if (tile > 0)
      {
        sprites.drawOverwrite(ix * TILE_WIDTH - cameraX, iy * TILE_HEIGHT, tileset, tile - 1);
      }
    }
  }
}

