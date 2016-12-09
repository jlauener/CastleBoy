#include "global.h"

#include "menu.h"
#include "game.h"

void setup()
{
  ab.begin();
  ab.setFrameRate(FPS);

  Menu::showTitle();
}

void loop()
{
  if (!ab.nextFrame())
  {
    return;
  }

  ab.pollButtons(); 
  ab.clear();

  if(mainState == STATE_GAME)
  {
    Game::loop();
  }
  else
  {
    Menu::loop();
  }
  
#ifdef DEBUG
  drawDebug();
#endif

  if(flashCounter > 0)
  {
    ab.fillRect(0, 0, 128, 64);
    flashCounter--;
  }
  ab.display();
}
