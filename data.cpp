#include "data.h"

#include "assets.h"

#define X 0,
#define G 1,
#define B 2,
#define C 3,

PROGMEM const uint8_t map_test[] = {
  // width, height
  32, 6,
  X X X X X X X X X X X X X X X C X X X X B B B X X X C X X X X X
  X X X X X X X C X X X X X X X X X X X X X X X X X X X X X X C X
  X X X X C X X X X X X X X X B B B B X X X X X X X X X X X X X X
  X X X X X X G G G X X X B B X X X B B X X X X X X X X X X X X X
  X X X G G G G G G G X X X X X X C B B B X X X X X X X X X X X X
  G G G G G G G G G G G G G G G G G G G G G G X X G G X X G G G G
};

const uint8_t* Data::mapData()
{
  return map_test;
}

