#include "TurboCore_SDL.h"

#include <SDL.h>
#include <SDL_audio.h>

#include <exception>
#include <string>
#include <map>
#include <cstdlib> // srand, rand
#include <ctime> // time

#define WINDOW_TITLE "CastleBoy"
#define WINDOW_SCALE 4

#define THROW_SDL_ERROR(msg) throw std::exception((std::string(msg) + " Error: " + SDL_GetError()).c_str())

bool quit = false;

SDL_Window * window;
SDL_Renderer* renderer;
SDL_Texture* screenBuffer;
SDL_Rect screenDestRect;

#define BEEPER_AMPLITUDE 1000
#define BEEPER_FREQUENCY 22050
SDL_AudioDeviceID beeper;
uint16_t toneCounter;
uint16_t toneFrequ;

void beeperCallback(void *_unused, uint8_t *_stream, int _length)
{
  int16_t* stream = (int16_t*)_stream;
  int length = _length / 2; // stream is 16bit

  for (int i = 0; i < length; i++)
  {
    if (toneCounter == 0)
    {
      // TODO play next tone if needed
    }

    if (toneCounter > 0)
    {
      bool low = (toneCounter / toneFrequ) % 2;
      stream[i] = low ? -BEEPER_AMPLITUDE : BEEPER_AMPLITUDE;
      toneCounter--;
    }
    else
    {
      stream[i] = 0;
    }
  }
}

std::map<SDL_Keycode, uint8_t> buttonMapping;
uint8_t buttonState;

uint32_t millis()
{
  return SDL_GetTicks();
}

uint32_t micros()
{
  return SDL_GetTicks() * 1000;
}

int SDLrun(void(&setup)(), void(&loop)())
{
  setup();

  while (!quit)
  {
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
      switch (evt.type)
      {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYDOWN:
      {
        auto button = buttonMapping.find(evt.key.keysym.sym);
        if (button != buttonMapping.end())
        {
          buttonState |= button->second;
        }
        break;
      }
      case SDL_KEYUP:
      {
        auto button = buttonMapping.find(evt.key.keysym.sym);
        if (button != buttonMapping.end())
        {
          buttonState &= ~button->second;
        }
        break;
      }
      }
    }

    loop();
  }

  if (screenBuffer != NULL)
  {
    SDL_DestroyTexture(screenBuffer);
  }

  if (renderer != NULL)
  {
    SDL_DestroyRenderer(renderer);
  }

  if (window != NULL)
  {
    SDL_DestroyWindow(window);
  }

  if (beeper != NULL)
  {
    SDL_CloseAudioDevice(beeper);
  }

  SDL_Quit();
  return 0;
}

void core::boot()
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
  {
    THROW_SDL_ERROR("SDL_Init");
  }

  // display
  int windowWidth = (WIDTH + 2) * WINDOW_SCALE;
  int windowHeight = (HEIGHT + 2) * WINDOW_SCALE;
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
  if (window == nullptr)
  {
    SDL_Quit();
    THROW_SDL_ERROR("SDL_CreateWindow");
    return;
  }

  screenDestRect.x = WINDOW_SCALE;
  screenDestRect.y = WINDOW_SCALE;
  screenDestRect.w = windowWidth - 2 * WINDOW_SCALE;
  screenDestRect.h = windowHeight - 2 * WINDOW_SCALE;

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr)
  {
    SDL_DestroyWindow(window);
    SDL_Quit();
    THROW_SDL_ERROR("SDL_CreateRenderer");
  }

  screenBuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (screenBuffer == nullptr)
  {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    THROW_SDL_ERROR("SDL_CreateTexture");
  }

  // input

  buttonMapping[SDLK_LEFT] = LEFT_BUTTON;
  buttonMapping[SDLK_a] = LEFT_BUTTON;

  buttonMapping[SDLK_RIGHT] = RIGHT_BUTTON;
  buttonMapping[SDLK_d] = RIGHT_BUTTON;

  buttonMapping[SDLK_UP] = UP_BUTTON;
  buttonMapping[SDLK_w] = UP_BUTTON;

  buttonMapping[SDLK_DOWN] = DOWN_BUTTON;
  buttonMapping[SDLK_s] = DOWN_BUTTON;

  buttonMapping[SDLK_LCTRL] = A_BUTTON;
  buttonMapping[SDLK_RCTRL] = A_BUTTON;
  buttonMapping[SDLK_LALT] = B_BUTTON;
  buttonMapping[SDLK_RALT] = B_BUTTON;

  // audio

  SDL_AudioSpec beeperSpec;
  beeperSpec.freq = BEEPER_FREQUENCY;
  beeperSpec.format = AUDIO_S16SYS;
  beeperSpec.channels = 1;
  beeperSpec.samples = 2048;
  beeperSpec.callback = beeperCallback;
  beeperSpec.userdata = NULL;  
  
  beeper = SDL_OpenAudioDevice(NULL, 0, &beeperSpec, NULL, 0);
  if (beeper == NULL)
  {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    THROW_SDL_ERROR("SDL_OpenAudioDevice");
  }

  SDL_PauseAudioDevice(beeper, false);

  // random

  srand((unsigned int)time(NULL));
}

uint8_t core::buttonsState()
{
  return buttonState;
}

void setPixel(void* screenPixels, int x, int y, uint8_t color)
{
  ((uint16_t*)screenPixels)[y * WIDTH + x] = color == 0 ? 0x0000 : 0xFFFF;
}

void core::paintScreen(uint8_t image[])
{
  int pitch;
  void* screenPixels;
  SDL_LockTexture(screenBuffer, NULL, &screenPixels, &pitch);

  for (int ix = 0; ix < WIDTH; ix++)
  {
    // set all pixels in column
    for (int iy = 0; iy < HEIGHT; iy++)
    {
      uint8_t row = image[(iy / 8) * WIDTH + ix];
      uint8_t bitMask = 0x01 << (iy % 8);
      setPixel(screenPixels, ix, iy, row & bitMask);
    }

    // clear the column
    for (int iy = 0; iy < HEIGHT / 8; iy++)
    {
      image[iy * WIDTH + ix] = 0x00;
    }
  }

  SDL_UnlockTexture(screenBuffer);
  SDL_Rect dest;
  dest.x = WINDOW_SCALE;
  dest.y = WINDOW_SCALE;  
  SDL_RenderCopy(renderer, screenBuffer, NULL, &screenDestRect);
  SDL_RenderPresent(renderer);
}

void core::setRGBled(uint8_t red, uint8_t green, uint8_t blue)
{
  std::cout << "setting RGB led to " << red << "," << green << "," << blue << std::endl;
}

uint8_t core::getRandomByte(uint16_t max)
{
  return rand() % max;
}

void core::tone(uint16_t frequency, uint16_t duration, void(*callback)())
{
  SDL_LockAudioDevice(beeper);
  toneCounter = duration;
  toneFrequ = frequency;
  SDL_UnlockAudioDevice(beeper);
}