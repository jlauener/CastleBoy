#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduboy2.h>
#include "game.h"

extern Arduboy2 ab;
extern Sprites sprites;
extern uint8_t gameState;
extern int16_t cameraX;

//#define DEBUG // enable to show log and CPU load
//#define DEBUG_HITBOX // enable to show hitboxes

#define FPS 60

#define CANDLE_MAX 16
#define ENEMY_MAX 16

#define STATE_GAME 0

// divider for fixed fractional numbers
#define F_PRECISION 1000

// player physic
#define PLAYER_JUMP_GRAVITY_F 190 // 0.18
#define PLAYER_FALL_GRAVITY_F 190 // 0.18
#define PLAYER_JUMP_FORCE_F 3100 // 3.0
#define PLAYER_JUMP_LEVITATE 3;

// enemy types
#define ENEMY_SKELETON 0

// size stuffs
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define HALF_TILE_WIDTH 4
#define HALF_TILE_HEIGHT 4
#define MAP_WIDTH_MAX 32
#define MAP_HEIGHT_MAX 6

#define CAMERA_BUFFER 32

#define SOLID_TILE_COUNT 3

// anim mode
#define ONE_SHOT 0
#define LOOP 1

// anim data indexes
#define MODE 0
#define FRAME_RATE 1
#define FRAME_COUNT 2
#define FRAMES 3

namespace Util
{
  bool collideHLine(int16_t lx, int16_t ly, int16_t lw, int16_t x, int16_t y, const Rect& hitbox);
}

#ifdef DEBUG
extern bool hasDebugValue;
extern int16_t debugValue;
extern const char* debugText;

#define LOG_DEBUG(x) logDebug(x)

void logDebug(const char* text);
void logDebug(int16_t value);
void drawDebug();
#else
#define LOG_DEBUG(x)
#endif


#endif
