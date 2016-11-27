#include "data.h"

#include "assets.h"

EntityData Data::player;

#define X 0,
#define G 1,
#define B 2,
//#define S 3,
#define C 4,

//PROGMEM const uint8_t map_test[] = {
//  // width, height
//  32, 6,
//  X X X X X X X X X X X X X X X X X X X X X X X X X X X G G G G G
//  X X X X X X X X X X X X X X X X X X X X X X X X X X G G G G G G
//  X X X X X X X X X X X X X X X X X X X B B X X X X G G G G G G G
//  X X X C X X X X X X X X X X X X X X X X X X X X G G G G G G G G
//  X X X X X X X X B B X X B B X X G G G G G G G G G G G G G G G G
//  G G G G G G X X X X X X X X X X G G G G G G G G G G G G G G G G
//};


PROGMEM const uint8_t map_test[] = {
  // width, height
  32, 6,
  X X X X X X X X X X X X X X X X X X X X X X X X X X X G G G G G
  X X X X X X X X X X X X X X X X X X X X X X X X X X G G G G G G
  X X X X X X X X X C X X X X X X X X X B B X X X X G G G G G G G
  X X X X X C X X B B X X X X X X X X X X X X X X G G G G G G G G
  X X X X B B X X X C X X X X X X G G G G G G G G G G G G G G G G
  G G G G G G G G G G G G G G G G G G G G G G G G G G G G G G G G
};

void Data::init()
{
//  player.width = 8;
//  player.height = 14;
//  player.originX = 4;
//  player.originY = 14;
  player.sprite = player_plus_mask;
  player.flippedFrameOffset = 8;
}

const uint8_t* Data::mapData()
{
  return map_test;
}

