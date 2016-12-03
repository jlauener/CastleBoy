#include "game.h"

#include "global.h"
#include "assets.h"
#include "map.h"
#include "player.h"
#include "entity.h"

void Game::init()
{
  Entities::init();
  Map::init(test);
  Player::init(8, 56);
}

void Game::loop()
{
  if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
  {
    Game::init();
    return;
  }

  Player::update();
  Entities::update();

  int backgroundOffset = 8 - cameraX / 8; // FIXME properly calculate parralax
  sprites.drawOverwrite(16 + backgroundOffset, 4, background_mountain, 0);
  Map::draw();
  Entities::draw();
  Player::draw();
  
  for(uint8_t i = 0; i < playerHp; i++)
  {
    sprites.drawPlusMask(i * 7, 0, ui_heart_plus_mask, 0);
  }
}

