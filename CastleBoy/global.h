#ifndef GLOBAL_H
#define GLOBAL_H

#include "Arglib.h"
#include "game.h"

extern Arduboy ab;
extern Sprites sprites;
extern ArduboyTones sound;
extern uint8_t mainState;
extern uint8_t flashCounter;

//#define DEBUG_LOG // enable to show debug logs (LOG_DEBUG)
//#define DEBUG_CPU // enable to display cpu load
#define DEBUG_CHEAT // enable to reset level with A+B+down
//#define DEBUG_HITBOX // enable to show hitboxes

#define FPS 60

#define ENTITY_MAX 32

#define STATE_TITLE 0
#define STATE_STAGE_INTRO 1
#define STATE_GAME 2
#define STATE_GAME_OVER 3

// divider for fixed fractional numbers
#define F_PRECISION 1000

// player
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
#define STARTING_LIFE 3
#define STARTING_HP 5

// map data
#define TILE_DATA_EMPTY 0
#define TILE_DATA_PROP 1
#define TILE_DATA_MISC 2
#define TILE_DATA_MAIN 3

// tile sprite
#define TILE_WALL 0
#define TILE_WALL_ALT 1
#define TILE_WALL_END 2
#define TILE_GROUND_START 3
#define TILE_GROUND 4
#define TILE_GROUND_START_ALT 5
#define TILE_GROUND_ALT 6
#define TILE_GRAVE 7
#define TILE_CHAIN 8
#define TILE_WINDOW 9

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

#define FONT_PAD 4 // actual width + padding

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

namespace Util
{
  bool collideRect(int16_t x1, int8_t y1, uint8_t width1, uint8_t height1, int16_t x2, int8_t y2, uint8_t width2, uint8_t height2);
}

#ifdef DEBUG_LOG
extern int16_t debugValue;
#define LOG_DEBUG(x) debugValue = x
void drawDebugLog();
#else
#define LOG_DEBUG(x)
#endif

#ifdef DEBUG_CPU
void drawDebugCpu();
#endif

#endif
