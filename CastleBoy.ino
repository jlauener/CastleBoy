#include "global.h"

#include "game.h"

typedef void (*FunctionPointer) ();

const FunctionPointer PROGMEM stateLoopFunction[] = {
  Game::loop  
};

void setup()
{
  ab.begin();
  ab.setFrameRate(FPS);

  Game::init();
  gameState = STATE_GAME;
}

void loop()
{
  if (!ab.nextFrame())
  {
    return;
  }

  ab.pollButtons(); 
  ab.clear();

  ((FunctionPointer) pgm_read_word (&stateLoopFunction[gameState]))();  
  
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
