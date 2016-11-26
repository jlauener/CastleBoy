#include "data.h"

#include "assets.h"

EntityData Data::player;

void Data::init()
{
  player.width = 8;
  player.height = 14;    
  player.originX = 4;
  player.originY = 14;
  player.sprite = player_plus_mask;
  player.flippedFrameOffset = 8;
}

