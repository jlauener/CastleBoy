#include "game_scene.h"

#include "assets.h"
#include "map.h"
#include "player.h"

void GameScene::init()
{
  Map::init();
  Player::init(40, 56);
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
   for (uint8_t i = 0u; i < 16u; i++)
  {
    // FIXME
    sprites.drawOverwrite(i * 8u, 0, background_bottom, i);
    sprites.drawOverwrite(i * 8u, 8, background_top, i);
  }
  Map::draw();
  Player::draw();
}

