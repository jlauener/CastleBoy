#include "candle.h"

#include "assets.h"

namespace
{
struct Candle
{
  int16_t x;
  int16_t y;
  // TODO use bitmask
  bool active;
  uint8_t frame;
};

Candle candles[CANDLE_MAX];
} // unamed

void Candles::init()
{
  for (byte i = 0; i < CANDLE_MAX; i++)
  {
    candles[i].active = false;
  }
}

void Candles::add(int x, int y)
{
  for (byte i = 0; i < CANDLE_MAX; i++)
  {
    if (!candles[i].active)
    {
      candles[i].x = x;
      candles[i].y = y;
      candles[i].active = true;
      candles[i].frame = 0;
      return;
    }
  }

  // FIXME assert?
}

bool collideHLineRect(int lx, int ly, int lw, int rx, int ry, int rw, int rh)
{
  if (lx + lw < rx || lx > rx + rw)
  {
    return false;
  }

  return ly >= ry && ly <= ry + rh;
}

void Candles::hit(int x, int y, int w)
{
  for (byte i = 0; i < CANDLE_MAX; i++)
  {
    if (candles[i].active)
    {
      if (collideHLineRect(x, y, w, candles[i].x + 1, candles[i].y + 1, 6, 6))
      {
        candles[i].active = false;
      }
    }
  }
}

void Candles::draw()
{
  for (byte i = 0; i < CANDLE_MAX; i++)
  {
    if (candles[i].active)
    {
      if (ab.everyXFrames(8))
      {
        ++candles[i].frame %= 3;
      }
      sprites.drawPlusMask(candles[i].x - cameraX, candles[i].y - 2, candle_plus_mask, candles[i].frame);
    }
  }
}
