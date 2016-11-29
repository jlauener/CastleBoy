#include "game.h"

#include "data.h"
#include "assets.h"
#include "map.h"
#include "player.h"
#include "candle.h"
#include "enemy.h"

void Game::init()
{
  Candles::init();
  Enemies::init();
  Map::init(Data::mapData());
  Player::init(10, 0);
}

void Game::loop()
{
  if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
  {
    Game::init();
    return;
  }

  Player::update();
  Enemies::update();

  int backgroundOffset = 8 - cameraX / 8; // FIXME properly calculate parralax
  sprites.drawOverwrite(16 + backgroundOffset, 0, background, 0);
  Map::draw();
  Candles::draw();
  Enemies::draw();
  Player::draw();
}

