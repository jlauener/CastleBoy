#include "map.h"

#include "assets.h"
#include "entity.h"

// map data
#define TILE_DATA_EMPTY 0
#define TILE_DATA_PROP 1
#define TILE_DATA_WALL 2
#define TILE_DATA_GROUND 3

// tile sprite
#define TILE_WALL 0
#define TILE_WALL_END 1
#define TILE_GROUND_START 2
#define TILE_GROUND 3
#define TILE_GROUND_START_ALT 4
#define TILE_GROUND_ALT 5
#define TILE_PROP 6

uint8_t Map::width;

namespace
{
const uint8_t* tilemap;
uint8_t height;

uint8_t getTileAt(uint8_t x, uint8_t y)
{
  return (pgm_read_byte(tilemap + (x * height + y) / 4) & (0x03 << (y % 4) * 2)) >> (y % 4) * 2;
}

}  // unamed

void Map::init(const uint8_t* source)
{
  width = pgm_read_byte(source);
  height = pgm_read_byte(++source);
  tilemap = ++source;
  Game::cameraX = 0;

  // load entities
  source += width * height / 4;
  uint8_t entityCount = pgm_read_byte(source);
  for (uint8_t i = 0; i < entityCount; i++)
  {
    uint8_t temp = pgm_read_byte(++source);
    uint8_t entityType = (temp & 0xF0) >> 4;
    uint8_t y = temp & 0x0F;
    uint8_t x = pgm_read_byte(++source);
    Entities::add(entityType, x, y);
  }
}

// FIXME can we use uint8_t instead of int16_t ?
bool Map::collide(int16_t x, int8_t y, const Box& hitbox)
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
      if (getTileAt(ix, iy) != TILE_DATA_EMPTY && getTileAt(ix, iy) != TILE_DATA_PROP)
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

bool Map::moveY(Vec& pos, int8_t dy, const Box& hitbox)
{
  if (dy != 0)
  {
    int8_t sign = dy > 0 ? 1 : -1;
    while (dy != 0)
    {
      if (Map::collide(pos.x, pos.y + sign, hitbox))
      {
        return true;
      }
      pos.y += sign;
      dy -= sign;
    }
  }

  return false;
}

// can be optimized CPU wise if needed
void Map::draw()
{
  uint8_t start = Game::cameraX / 8;

  for (uint8_t ix = start; ix < start + 17; ix++)
  {
    bool isGround = false;
    bool isWall = false;
    for (uint8_t iy = 0; iy < height; iy++)
    {
      uint8_t tile = getTileAt(ix, iy);
      if (tile == TILE_DATA_EMPTY)
      {
        if (isWall)
        {
          isWall = false;
          tile = TILE_WALL_END;
        }
        else
        {
          continue;
        }
      }
      else if (tile == TILE_DATA_WALL)
      {
        tile = TILE_WALL;
        isWall = true;
        isGround = false;
      }
      else if (tile == TILE_DATA_GROUND)
      {
        bool useAlt = ix % 2 == 0 && iy % 2 == 1 || ix % 2 == 1 && iy & 2 == 0;
        if (isGround)
        {
          // already in ground, use inner ground tile
          tile = useAlt ? TILE_GROUND_ALT : TILE_GROUND;
        }
        else
        {
          // first ground tile
          tile = useAlt ? TILE_GROUND_START_ALT : TILE_GROUND_START;
          isGround = true;
        }
        isWall = false;
      }
      else // tile == TILE_DATA_PROP
      {
        tile = TILE_PROP;
      }

      sprites.drawOverwrite(ix * TILE_WIDTH - Game::cameraX, iy * TILE_HEIGHT, tileset, tile);
    }
  }
}

