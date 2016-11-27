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
}

void Candles::clear()
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

  LOG_WARNING("no free candles");
}

void Candles::hit(int x, int y, int w)
{
  for (byte i = 0; i < CANDLE_MAX; i++)
  {
    if (candles[i].active)
    {
      // hitbox 4x4
      // left x + 2
      // right x + 6
      // top y + 4
      // bottom y + 8
      if(candles[i].x > x && candles[i].x + 8 < x + w && candles[i].y + 4 < y && y < candles[i].y + 8)
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
      if(ab.everyXFrames(10))
      {
        ++candles[i].frame %= 3;
      }
      sprites.drawOverwrite(candles[i].x - cameraX, candles[i].y, candle, candles[i].frame);
    }
  }
}
