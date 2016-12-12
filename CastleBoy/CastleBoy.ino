#include "global.h"

#include "menu.h"
#include "game.h"

void setup()
{
  ab.start();
  ab.setFrameRate(FPS);

  Menu::showTitle();
}

void loop()
{
  if (!ab.nextFrame())
  {
    return;
  }

  ab.poll();
  ab.clearDisplay();
  
  Menu::loop();

#ifdef DEBUG_LOG
  drawDebugLog();
#endif

#ifdef DEBUG_CPU
  drawDebugCpu();
#endif

  if (flashCounter > 0)
  {
    ab.fillRect(0, 0, 128, 64, WHITE);
    flashCounter--;
  }
  ab.display();
}
