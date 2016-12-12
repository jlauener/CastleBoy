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


// Inspired by TEAMArg's Sirene, stages.h:775
// But optimized (use of int8_t, use cast instead of for loop)
void Util::drawNumber(int16_t x, int16_t y, uint16_t value, uint8_t zeroPad)
{
  char buf[10];
  ltoa(value, buf, 10);
  uint8_t strLength = strlen(buf);
  uint8_t pad = zeroPad > strLength ? zeroPad - strLength : 0;

  // draw 0 padding
  for (uint8_t i = 0; i < pad; i++)
  {
    sprites.drawSelfMasked(x + FONT_PAD * i, y, font, 0);
  }

  // draw the number
  for (uint8_t i = 0; i < strLength; i++)
  {
    uint8_t digit = (uint8_t) buf[i];
    digit -= 48;
    if (digit > 9) digit = 0;
    sprites.drawSelfMasked(x + pad * FONT_PAD + FONT_PAD * i, y, font, digit);
  }
}

int freeRam()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

#ifdef DEBUG_LOG
#include "menu.h"
int16_t debugValue = 0;
void drawDebugLog()
{
  ab.fillRect(0, 0, 30, 8, BLACK);
  Util::drawNumber(0, 0, debugValue);
}
#endif

#ifdef DEBUG_CPU
#include "menu.h"
void drawDebugCpu()
{
  ab.fillRect(100, 0, 30, 8, BLACK);
  Util::drawNumber(110, 0, ab.cpuLoad());
}
#endif

#ifdef DEBUG_RAM
#include "menu.h"
void drawDebugRam()
{
  extern int __heap_start, *__brkval;
  int v;
  int ram = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);

  ab.fillRect(60, 0, 30, 8, BLACK);
  Util::drawNumber(60, 0, ram);
}
#endif
