#ifndef __ENGINE_H__
#define __ENGINE_H__

#include "Arduboy2.h"

class Scene
{
  public:
    virtual void update() = 0;
    virtual void draw() = 0;
};

#define LOG_DEBUG(x) Engine::log(x)
#define LOG_WARNING(x) Engine::log(x) // TODO

namespace Engine
{
void setup(uint8_t frameRate);
void loop();

void show(Scene& scene);

void log(const char* text);
void log(int16_t value);
}

#endif


