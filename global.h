#ifndef GLOBAL_H
#define GLOBAL_H

#include <Arduboy2.h>
#include <ArduboyTones.h>
#include "game.h"

extern Arduboy2 ab;
extern ArduboyTones sound;
extern Sprites sprites;
extern uint8_t gameState;
extern uint8_t flashCounter;

struct Vec
{
  int16_t x;
  int8_t y;
};

struct Box
{
  int16_t x;
  int8_t y;
  uint8_t width;
  uint8_t height;
};

//#define DEBUG // enable to show log and CPU load
//#define DEBUG_HITBOX // enable to show hitboxes

#define FPS 60

#define ENTITY_MAX 32

#define STATE_GAME 0

// divider for fixed fractional numbers
#define F_PRECISION 1000

// player physic
#define PLAYER_JUMP_GRAVITY_F 190 // 0.18
#define PLAYER_FALL_GRAVITY_F 190 // 0.18
#define PLAYER_JUMP_FORCE_F 3100 // 3.0
#define PLAYER_LEVITATE_DURATION 3
#define PLAYER_KNOCKBACK_DURATION 24
#define PLAYER_KNOCKBACK_FAST 18
#define PLAYER_SPEED_NORMAL 2 // every 2 frames
#define PLAYER_SPEED_DUCK 4 // every 4 frames
#define PLAYER_SPEED_KNOCKBACK_NORMAL 2 // every 2 frames
#define PLAYER_SPEED_KNOCKBACK_FAST 1 // every 1 frame

// entity types
#define ENTITY_CANDLE 0
#define ENTITY_SKELETON 1
#define ENTITY_SKULL 2
#define ENTITY_COIN 3

// entity score
#define SCORE_CANDLE 5
#define SCORE_SKELETON 10
#define SCORE_SKULL 20
#define SCORE_COIN 50

// size stuffs
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define HALF_TILE_WIDTH 4
#define HALF_TILE_HEIGHT 4
#define MAP_WIDTH_MAX 16
#define MAP_HEIGHT_MAX 6

#define CAMERA_BUFFER 64

#define SOLID_TILE_COUNT 3

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
