#include "game.h"

#include "global.h"
#include "assets.h"
#include "map.h"
#include "player.h"
#include "entity.h"

int16_t cameraX;

void Game::init()
{
  Player::hp = 5;
  Player::score = 0;
  
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
  
  for(uint8_t i = 0; i < Player::hp; i++)
  {
    sprites.drawPlusMask(i * 7, 0, ui_heart_plus_mask, 0);
  }

  // FIXME
  uint16_t n;
  if(Player::score >= 1000) n = 4;
  else if(Player::score >= 100) n = 3;
  else if(Player::score >= 10) n = 2;
  else n = 1;
  ab.setCursor(128 - n * 6, 0);
  ab.print(Player::score);
}

