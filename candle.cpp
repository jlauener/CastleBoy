#include "candle.h"

#include "assets.h"

// TODO merge with enemy and create generic entity!
namespace
{

Point spriteOrigin =
{
  4, 10
};

Rect candleHitbox =
{
  2, 8, // x, y
  4, 6, // width, height
};

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

void Candles::hit(int x, int y, int w)
{
  for (byte i = 0; i < CANDLE_MAX; i++)
  {
    if (candles[i].active)
    {
      if(Util::collideHLine(x, y, w, candles[i].x, candles[i].y, candleHitbox))
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
      sprites.drawPlusMask(candles[i].x - spriteOrigin.x - cameraX, candles[i].y - spriteOrigin.y, candle_plus_mask, candles[i].frame);

#ifdef DEBUG_HITBOX
     ab.fillRect(candles[i].x - candleHitbox.x - cameraX, candles[i].y - candleHitbox.y, candleHitbox.width, candleHitbox.height);
#endif
    }
  }
}
