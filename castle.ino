#include "global.h"
#include "data.h"

// MVP
// TODO: scrolling
// TODO: experiment with 8x4 tiles?
// TODO: attack impl
// TODO: candle impl

void setup()
{
  Engine::setup(60);

  Data::init();
  gameScene.init();
  Engine::show(gameScene);
}

void loop()
{
  Engine::loop();
}
