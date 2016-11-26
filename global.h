#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include "Arduboy2.h"

#include "game_scene.h"

extern Arduboy2 ab;
extern Sprites sprites;
extern GameScene gameScene;

extern int16_t cameraX;

// divider for fixed fractional numbers
#define F_PRECISION 1000

// player physic
#define PLAYER_JUMP_GRAVITY_F 180 // 0.18
#define PLAYER_FALL_GRAVITY_F 180 // 0.18
#define PLAYER_JUMP_FORCE_F 3000 // 3.0
#define PLAYER_JUMP_LEVITATE 3;

// size stuffs
#define TILE_SIZE 8
#define MAP_HEIGHT 8
#define CAMERA_BUFFER 32

#define SOLID_TILE_COUNT 1

// anim mode
#define ONE_SHOT 0
#define LOOP 1

// anim data indexes
#define MODE 0
#define FRAME_RATE 1
#define FRAME_COUNT 2
#define FRAMES 3

#endif
