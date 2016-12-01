#include "global.h"

Arduboy2 ab;
Sprites sprites;
ArduboyTones sound(ab.audio.enabled);
uint8_t gameState;
int16_t cameraX = 0;

bool Util::collideHLine(int16_t lx, int16_t ly, int16_t lw, int16_t x, int16_t y, const Rect& hitbox)
{
  x -= hitbox.x;
  y -= hitbox.y;

  if (lx + lw < x || lx > x + hitbox.width)
  {
    return false;
  }

  return ly >= y && ly <= y + hitbox.height;
}

bool Util::collide(int16_t x1, int16_t y1, const Rect& hitbox1, int16_t x2, int16_t y2, const Rect& hitbox2)
{
  x1 -= hitbox1.x;
  y1 -= hitbox1.y;
  x2 -= hitbox2.x;
  y2 -= hitbox2.y;

  return !(x2                  >= x1 + hitbox1.width  ||
           x2 + hitbox2.width  <= x1                ||
           y2                  >= y1 + hitbox1.height ||
           y2 + hitbox2.height <= y1);
}

#ifdef DEBUG
bool hasDebugValue = false;
int16_t debugValue = 0;
const char* debugText = NULL;

void logDebug(const char* text)
{
  hasDebugValue = false;
  debugText = text;
}

void logDebug(int16_t value)
{
  hasDebugValue = value;
  debugValue = value;
}

void drawDebug()
{
  if (hasDebugValue)
  {
    ab.setCursor(0, 0);
    ab.print(debugValue);
  }
  else if (debugText != NULL)
  {
    ab.setCursor(0, 0);
    ab.print(debugText);
  }

  ab.setCursor(110, 0);
  ab.print(ab.cpuLoad());
}
#endif

