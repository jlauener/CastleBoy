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
  //int backgroundOffset = (((cameraX * 1000) / ((Map::width() * TILE_WIDTH) / 1000)) - 500) / 20;
  int backgroundOffset = 8 - cameraX / 8; // FIXME
  for (uint8_t i = 0; i < 16; i++)
  {
    // FIXME
    sprites.drawOverwrite(backgroundOffset + i * 8, 0, background_bottom, i);
    sprites.drawOverwrite(backgroundOffset + i * 8, 8, background_top, i);
  }
  Map::draw();
  Candles::draw();
  Player::draw();
}

