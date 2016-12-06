#include "game.h"

#include "global.h"
#include "assets.h"
#include "map.h"
#include "player.h"
#include "entity.h"

int16_t cameraX;

namespace
{
uint8_t deathCounter = 0;
uint8_t shakeCounter = 0;
int8_t shakeStrenght = 0;
}

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

  // check if stage is finished
  if (Player::pos.x - 4 /*normalHitbox.x*/ > Map::width * TILE_WIDTH)
  {
    Game::init(); // TODO
  }

  // check if player is dead
  if (deathCounter == 0 && !Player::alive)
  {
    deathCounter = 120;
  }

  if (deathCounter > 0)
  {
    if (--deathCounter == 0)
    {
      Game::init(); // TODO
    }
  }

  // update camera
  if (Player::pos.x < cameraX + CAMERA_BUFFER)
  {
    cameraX = Player::pos.x - CAMERA_BUFFER;
    if (cameraX < 0) cameraX = 0;
  }
  else if (Player::pos.x > cameraX + 128 - CAMERA_BUFFER)
  {
    cameraX = Player::pos.x - 128 + CAMERA_BUFFER;
    if (cameraX > Map::width * TILE_WIDTH - 128) cameraX = Map::width * TILE_WIDTH - 128;
  }

  if (shakeCounter > 0)
  {
    cameraX += shakeStrenght;
    if (shakeCounter % 4 == 0)
    {
      shakeStrenght = -shakeStrenght;
    }
    shakeCounter--;
  }

  int16_t backgroundOffset = cameraX / 28; // FIXME properly calculate parralax unless all maps have same width
  sprites.drawOverwrite(16 - backgroundOffset, 4, background_mountain, 0);
  Map::draw();
  Entities::draw();
  Player::draw();

  for (uint8_t i = 0; i < Player::hp; i++)
  {
    sprites.drawPlusMask(i * 7, 0, ui_heart_plus_mask, 0);
  }

  // FIXME
  uint16_t n;
  if (Player::score >= 1000) n = 4;
  else if (Player::score >= 100) n = 3;
  else if (Player::score >= 10) n = 2;
  else n = 1;
  ab.setCursor(128 - n * 6, 0);
  ab.print(Player::score);
}

void Game::shake(uint8_t duration, int8_t strenght)
{
  shakeCounter = duration;
  shakeStrenght = strenght;
}

