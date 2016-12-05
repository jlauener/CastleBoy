#include "global.h"

Arduboy2 ab;
Sprites sprites;
ArduboyTones sound(ab.audio.enabled);
uint8_t gameState;

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

