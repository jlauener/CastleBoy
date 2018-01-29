#include "TurboCore_SDL.h"

#include <exception>
#include <string>
#include <map>
#include <cstdlib> // srand, rand
#include <ctime> // time

#define WINDOW_TITLE "CastleBoy"
#define WINDOW_SCALE 4
#define WINDOW_WIDTH WIDTH * WINDOW_SCALE
#define WINDOW_HEIGHT HEIGHT * WINDOW_SCALE

#define THROW_SDL_ERROR(msg) throw std::exception((std::string(msg) + " Error: " + SDL_GetError()).c_str())

bool quit = false;

SDL_Window * window;
SDL_Renderer* renderer;
SDL_Texture* screenBuffer;

std::map<SDL_Keycode, uint8_t> buttonMapping;
uint8_t buttonState;

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

  SDL_Quit();
  return 0;
}

void core::boot()
{
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    THROW_SDL_ERROR("SDL_Init");
  }

  // display

  window = SDL_CreateWindow(WINDOW_TITLE, 400, 400, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == nullptr)
  {
    SDL_Quit();
    THROW_SDL_ERROR("SDL_CreateWindow");
    return;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /* | SDL_RENDERER_PRESENTVSYNC*/);
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
  SDL_RenderCopy(renderer, screenBuffer, NULL, NULL);
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