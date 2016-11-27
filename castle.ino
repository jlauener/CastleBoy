#include "global.h"

void setup()
{
  Engine::setup(60);

  gameScene.init();
  Engine::show(gameScene);
}

void loop()
{
  Engine::loop();
}
