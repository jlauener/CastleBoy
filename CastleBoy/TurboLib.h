#ifndef TURBO_LIB_H
#define	TURBO_LIB_H

#ifdef SDL_TARGET
#include "TurboCore_SDL.h"
#else
#include "TurboCore.h"
#endif

typedef uint8_t color_t;

#define ALIGN_LEFT 0
#define ALIGN_CENTER 1
#define ALIGN_RIGHT 2

namespace ab
{
  // -----------------------------------------------------------------------
  // CORE
  // -----------------------------------------------------------------------
  void init(uint8_t frameRate);
  bool beginUpdate();
  void endUpdate();
  extern uint16_t frameCount;

  // -----------------------------------------------------------------------
  // INPUT
  // -----------------------------------------------------------------------
  bool isButtonDown(uint8_t button);
  bool wasButtonPressed(uint8_t button);
  bool wasButtonReleased(uint8_t button);

  // -----------------------------------------------------------------------
  // DISPLAY
  // -----------------------------------------------------------------------
  void setPixel(int16_t x, int16_t y, color_t color);
  color_t getPixel(uint8_t x, uint8_t y);

  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, color_t color);
  void drawVLine(int16_t x, int16_t y, uint8_t h, color_t color);
  void drawHLine(int16_t x, int16_t y, uint8_t w, color_t color);
  void drawRect(int16_t x, int16_t y, uint8_t w, uint8_t h, color_t color);
  void fillRect(int16_t x, int16_t y, uint8_t w, uint8_t h, color_t color);
  void fillScreen(color_t color);

  void drawOverwrite(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame = 0);
  void drawSelfMasked(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame = 0);
  void drawPlusMask(int16_t x, int16_t y, const uint8_t *bitmap, uint8_t frame = 0);

  void drawNumber(uint8_t x, uint8_t y, uint16_t value, uint8_t align = ALIGN_LEFT);

  // -----------------------------------------------------------------------
  // UTILS
  // -----------------------------------------------------------------------
  bool everyXFrames(uint8_t frame);
  void toggle(uint8_t & flags, uint8_t mask);
  bool collide(int16_t x1, int8_t y1, uint8_t w1, uint8_t h1, int16_t x2, int8_t y2, uint8_t w2, uint8_t h2);
};

#endif

