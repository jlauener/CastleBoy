#include "global.h"

#include "assets.h"

Arduboy ab;
Sprites sprites(ab);
ArduboyTones sound(ab.audio.enabled);
uint8_t mainState;
uint8_t flashCounter = 0;


bool Util::collideRect(int16_t x1, int8_t y1, uint8_t width1, uint8_t height1, int16_t x2, int8_t y2, uint8_t width2, uint8_t height2)
{

  return !(x1            >= x2 + width2  ||
           x1 + width1   <= x2           ||
           y1            >= y2 + height2 ||
           y1 + height1  <= y2);
}

#ifdef DEBUG_LOG
#include "menu.h"
int16_t debugValue = 0;
void drawDebugLog()
{
  ab.fillRect(0, 0, 30, 8, BLACK);
  Menu::drawNumber(0, 0, debugValue);
}
#endif

#ifdef DEBUG_CPU
#include "menu.h"
void drawDebugCpu()
{
  ab.fillRect(100, 0, 30, 8, BLACK);
  Menu::drawNumber(110, 0, ab.cpuLoad());
}
#endif
