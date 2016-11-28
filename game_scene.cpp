#include "game_scene.h"

#include "data.h"
#include "assets.h"
#include "map.h"
#include "player.h"
#include "candle.h"

void GameScene::init()
{
  Candles::clear();
  Map::init(Data::mapData());
  Player::init(10, 0);
}

void GameScene::update()
{
  if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
  {
    gameScene.init();
    return;
  }

  Player::update();
}

void GameScene::draw()
{
  int backgroundOffset = 8 - cameraX / 8; // FIXME properly calculate parralax
  sprites.drawOverwrite(16 + backgroundOffset, 0, background, 0);
  Map::draw();
  Candles::draw();
  Player::draw();
}

