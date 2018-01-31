#ifndef TURBO_CORE_SDL_H
#define TURBO_CORE_SDL_H

#define WIDTH 128
#define HEIGHT 64

#include <cstdint>
#include <iostream>

// maintain code compatiblity with Arduino lib
#define PROGMEM
#define pgm_read_byte(addr) (*((const uint8_t*)(addr)))
#define pgm_read_word(addr) (*((const uint16_t*)(addr)))

uint32_t millis();
uint32_t micros();

#define _BV(bit) (1 << (bit))
#define min(a, b) (a < b ? a : b)
#define max(a, b) (a > b ? a : b)
////

#ifdef LOG_ENABLED
#define LOG_DEBUG(msg) std::cout<<"DEBUG " <<msg << std::endl
#define LOG_INFO(msg) std::cout<<"INFO " <<msg << std::endl
#define LOG_WARN(msg) std::cout<<"WARN " <<msg << std::endl
#else
#define LOG_DEBUG(msg)
#define LOG_INFO(msg)
#define LOG_WARN(msg)
#endif

#define WHITE 1
#define BLACK 0

#define LEFT_BUTTON _BV(5)
#define RIGHT_BUTTON _BV(2)
#define UP_BUTTON _BV(4)
#define DOWN_BUTTON _BV(6)
#define A_BUTTON _BV(1)
#define B_BUTTON _BV(0)

int SDLrun(void(&setup)(), void(&loop)());

namespace core
{
  /** \brief
  * Initialize the Arduboy's hardware.
  *
  * \details
  * This function initializes the display, buttons, etc.
  *
  * This function is called by begin() so isn't normally called within a
  * sketch. However, in order to free up some code space, by eliminating
  * some of the start up features, it can be called in place of begin().
  * The functions that begin() would call after boot() can then be called
  * to add back in some of the start up features, if desired.
  * See the README file or documentation on the main page for more details.
  */
  void boot();

  /** \brief
   * Get the current state of all buttons as a bitmask.
   *
   * \return A bitmask of the state of all the buttons.
   *
   * \details
   * The returned mask contains a bit for each button. For any pressed button,
   * its bit will be 1. For released buttons their associated bits will be 0.
   *
   * The following defined mask values should be used for the buttons:
   *
   * LEFT_BUTTON, RIGHT_BUTTON, UP_BUTTON, DOWN_BUTTON, A_BUTTON, B_BUTTON
   */
  uint8_t buttonsState();

  /** \brief
   * Paints an entire image directly to the display from an array in RAM.
   *
   * \param image A byte array in RAM representing the entire contents of
   * the display.
   *
   * \details
   * The contents of the specified array in RAM is written to the display.
   * Each byte in the array represents a vertical column of 8 pixels with
   * the least significant bit at the top. The bytes are written starting
   * at the top left, progressing horizontally and wrapping at the end of
   * each row, to the bottom right. The size of the array must exactly
   * match the number of pixels in the entire display.
   */
  void paintScreen(uint8_t image[]);

  /** \brief
   * Set the light output of the RGB LED.
   *
   * \param red,green,blue The brightness value for each LED.
   *
   * \details
   * The RGB LED is actually individual red, green and blue LEDs placed
   * very close together in a single package. By setting the brightness of
   * each LED, the RGB LED can show various colors and intensities.
   * The brightness of each LED can be set to a value from 0 (fully off)
   * to 255 (fully on).
   *
   * \note
   * \parblock
   * Certain libraries that take control of the hardware timers may interfere
   * with the ability of this function to properly control the RGB LED.
   *_ArduboyPlaytune_ is one such library known to do this.
   * The digitalWriteRGB() function will still work properly in this case.
   * \endparblock
   *
   * \note
   * \parblock
   * Many of the Kickstarter Arduboys were accidentally shipped with the
   * RGB LED installed incorrectly. For these units, the green LED cannot be
   * lit. As long as the green led is set to off, setting the red LED will
   * actually control the blue LED and setting the blue LED will actually
   * control the red LED. If the green LED is turned fully on, none of the
   * LEDs will light.
   * \endparblock
   */
  void setRGBled(uint8_t red, uint8_t green, uint8_t blue);

  void tone(uint16_t frequency, uint16_t duration, void (*callback)());

  uint8_t getRandomByte(uint16_t max = 256);
};

#endif