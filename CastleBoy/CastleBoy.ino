#include "global.h"

#include "menu.h"
#include "game.h"
#include "assets.h"

uint8_t bootCounter = 0;

void setup()
{
  ab::init(FPS);

  Menu::showTitle();
}

void loop()
{
  if (!ab::beginUpdate())
  {
    return;
  }

  if(bootCounter < 120)
  {
    bootCounter++;
    ab::drawOverwrite(49, 14, logo, 0);
    ab::endUpdate();
    return;
  }

  Menu::loop();

#ifdef DEBUG_LOG
  drawDebugLog();
#endif

#ifdef DEBUG_CPU
  drawDebugCpu();
#endif

#ifdef DEBUG_RAM
  drawDebugRam();
#endif

  if (flashCounter > 0)
  {
    ab::fillRect(0, 0, 128, 64, WHITE);
    flashCounter--;
  }
  ab::endUpdate();
}
