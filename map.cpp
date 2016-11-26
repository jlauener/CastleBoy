#include "map.h"

#include "assets.h"

namespace
{
int16_t mapWidth = 44;
uint8_t data[] =
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 3, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
  2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 0, 0, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

}

void Map::init()
{
  cameraX = 12;
}

int16_t Map::width()
{
  return mapWidth;
}

bool Map::collide(int16_t x, int16_t y, int8_t w, int8_t h)
{
  if (x < 0 || x + w > mapWidth * TILE_SIZE)
  {
    // cannot get out on the sides, collide
    //LOG_DEBUG("side");
    return true;
  }
  
  int16_t tx1 = x / TILE_SIZE;
  int16_t ty1 = y / TILE_SIZE;
  int16_t tx2 = (x + w - 1) / TILE_SIZE;
  int16_t ty2 = (y + h - 1) / TILE_SIZE;

  if (ty2 < 0 || ty2 >= MAP_HEIGHT)
  {
    // either higher or lower than map, no collision
    //LOG_DEBUG("higher or lower");
    return false;
  }
  
  // clamp positions
  if (tx1 < 0) tx1 = 0;
  if (tx2 >= mapWidth) tx2 = mapWidth - 1;
  if (ty1 < 0) ty1 = 0;
  if (ty2 >= MAP_HEIGHT) ty2 = MAP_HEIGHT - 1;

  for (int16_t ix = tx1; ix <= tx2; ix++)
  {
    for (int16_t iy = ty1; iy <= ty2; iy++)
    {
      // FIXME
      if (data[iy * mapWidth + ix] > 0 && data[iy * mapWidth + ix] <= SOLID_TILE_COUNT)
      {
        // check for rectangle intersection
        if (ix * TILE_SIZE + TILE_SIZE > x && iy * TILE_SIZE + TILE_SIZE > y && ix * TILE_SIZE < x + w && iy * TILE_SIZE < y + h)
        {
          //LOG_DEBUG("hit");
          return true;
        }
      }
    }
  }
  
  return false;
}

void Map::draw()
{
  // TODO take into account cameraX
  for (int8_t ix = 0; ix < mapWidth; ix++)
  {
    for (int8_t iy = 0; iy < MAP_HEIGHT; iy++)
    {
      uint8_t tile = data[iy * mapWidth + ix];
      if (tile > 0)
      {
        //if (tile <= SOLID_TILE_COUNT)
        //{
          sprites.drawOverwrite(ix * TILE_SIZE - cameraX, iy * TILE_SIZE, tileset, tile - 1);
       // }
        //else
        //{
         // sprites.drawSelfMasked(ix * TILE_SIZE - cameraX, iy * TILE_SIZE, tileset, tile - 1);
        //}
      }
    }
  }
}

