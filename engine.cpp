#include "engine.h"

#include "global.h"

namespace
{
Scene* scene = NULL;
bool hasDebugValue = false;
int16_t debugValue = 0;
const char* debugText = NULL;
}

void Engine::setup(uint8_t frameRate)
{
  ab.begin();
  ab.setFrameRate(frameRate);
}

void Engine::loop()
{
  if (!ab.nextFrame())
  {
    return;
  }

  ab.pollButtons();
  scene->update();

  ab.clear();
  scene->draw();

//  if (hasDebugValue)
//  {
//    ab.setCursor(0, 0);
//    ab.print(debugValue);
//  }
//  else if (debugText != NULL)
//  {
//    ab.setCursor(0, 0);
//    ab.print(debugText);
//  }
//
//  ab.setCursor(110, 0);
//  ab.print(ab.cpuLoad());
  
  ab.display();
}

void Engine::show(Scene& scene_)
{
  scene = &scene_;
}

void Engine::log(const char* text)
{
  hasDebugValue = false;
  debugText = text;
}

void Engine::log(int16_t value)
{
  hasDebugValue = value;
  debugValue = value;
}
