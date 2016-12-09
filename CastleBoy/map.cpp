#include "map.h"

#include "assets.h"
#include "entity.h"
#include "menu.h"

uint8_t Map::width;

namespace
{
const uint8_t* tilemap;
uint8_t height;

uint8_t solidTileIndex;
uint8_t mainTile;
uint8_t mainTileAlt;
uint8_t mainStartTile;
uint8_t mainStartTileAlt;
uint8_t propTile;
bool endMiscTile;
uint8_t miscTile;
uint8_t miscEndTile;

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

  // TODO depend on map data
  switch (Menu::stageIndex)
  {
    case 0:
      solidTileIndex = 2;

      mainTile = TILE_GROUND;
      mainTileAlt = TILE_GROUND_ALT;
      mainStartTile = TILE_GROUND_START;
      mainStartTileAlt = TILE_GROUND_START_ALT;

      endMiscTile = true;
      miscTile = TILE_WALL;
      miscEndTile = TILE_WALL_END;

      propTile = TILE_GRAVE;
      break;
    case 1:
      solidTileIndex = 3;

      mainTile = TILE_WALL;
      mainTileAlt = TILE_WALL_ALT;
      mainStartTile = TILE_WALL;
      mainStartTileAlt = TILE_WALL_ALT;

      endMiscTile = false;
      miscTile = TILE_CHAIN;

      propTile = TILE_WINDOW;
      break;
    case 2:
      solidTileIndex = 2;

      mainTile = TILE_GROUND;
      mainTileAlt = TILE_GROUND_ALT;
      mainStartTile = TILE_GROUND_START;
      mainStartTileAlt = TILE_GROUND_START_ALT;

      endMiscTile = false;
      miscTile = TILE_WALL;
      miscEndTile = TILE_WALL_END;

      propTile = TILE_GRAVE;
      break;
    case 3:
      solidTileIndex = 3;

      mainTile = TILE_WALL;
      mainTileAlt = TILE_WALL_ALT;
      mainStartTile = TILE_WALL;
      mainStartTileAlt = TILE_WALL_ALT;

      endMiscTile = false;
      miscTile = TILE_CHAIN;

      propTile = TILE_WINDOW;
      break;
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
      if (getTileAt(ix, iy) >= solidTileIndex)
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

// can be optimized CPU wise if needed (by not using getTileAt)
void Map::draw()
{
  uint8_t start = Game::cameraX / 8;

  for (uint8_t ix = start; ix < start + 17; ix++)
  {
    bool isMain = false;
    bool needToEndMisc = false;
    for (uint8_t iy = 0; iy < height; iy++)
    {
      uint8_t tile = getTileAt(ix, iy);
      if (tile == TILE_DATA_EMPTY)
      {
        if (needToEndMisc)
        {
          tile = miscEndTile;
          needToEndMisc = false;
        }
        else
        {
          continue;
        }
      }
      else if (tile == TILE_DATA_MISC)
      {
        tile = miscTile;
        needToEndMisc = endMiscTile;
        isMain = false;
      }
      else if (tile == TILE_DATA_MAIN)
      {
        bool useAlt = ix % 2 == 0 && iy % 2 == 1 || ix % 2 == 1 && iy & 2 == 0;
        if (isMain)
        {
          // already started, use normal main tile
          tile = useAlt ? mainTileAlt : mainTile;
        }
        else
        {
          // first main tile from top
          tile = useAlt ? mainStartTileAlt : mainStartTile;
          isMain = true;
        }
        needToEndMisc = false;
      }
      else // tile == TILE_DATA_PROP
      {
        tile = propTile;
        needToEndMisc = false;
      }

      sprites.drawOverwrite(ix * TILE_WIDTH - Game::cameraX, iy * TILE_HEIGHT, tileset, tile);
    }
  }
}

