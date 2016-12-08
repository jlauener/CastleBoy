#include "game.h"

#include "assets.h"
#include "map.h"
#include "player.h"
#include "entity.h"
#include "menu.h"

namespace
{
uint8_t deathCounter = 0;
uint8_t life = 0;
uint8_t stageIndex;
bool restoreHp = false;
}

int16_t Game::cameraX;
uint16_t Game::score;

void Game::reset()
{
  stageIndex = 0;
  life = STARTING_LIFE;
  Player::hp = STARTING_HP;
  score = 0;
}

void Game::play()
{
  mainState = STATE_PLAY;
  if(restoreHp)
  {
    Player::hp = STARTING_HP;
    restoreHp = false;
  }
  Entities::init();
  Map::init(test);
  Player::init(8, 56);
}

void Game::loop()
{
// TODO debug stuffs
//  if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
//  {
//    Game::init();
//    return;
//  }

  Player::update();
  Entities::update();

  // check if stage is finished
  if (Player::pos.x - 4 /*normalHitbox.x*/ > Map::width * TILE_WIDTH)
  {
    if(++stageIndex == 3)
    {
      Menu::showGameOver();
    }
    else
    {
      Menu::showStageIntro();
    }
  }

  // check if player is dead
  if (deathCounter == 0 && !Player::alive)
  {
    deathCounter = 100;
    --life;
    restoreHp = true;
    sound.tone(NOTE_G3, 100, NOTE_G2, 150, NOTE_G1, 350);
  }

  if (deathCounter > 0)
  {
    if (--deathCounter == 0)
    {
      if(life == 0)
      {
        Menu::showGameOver();
      }
      else
      {
        Menu::showStageIntro();
      }
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

  int16_t backgroundOffset = cameraX / 28; // FIXME properly calculate parralax unless all maps have same width
  sprites.drawOverwrite(16 - backgroundOffset, 4, background_mountain, 0);
  Map::draw();
  Entities::draw();
  Player::draw();

  ab.fillRect(0, 0, FONT_PAD, 7, BLACK);
  Menu::drawNumber(0, 0, life);
  
  for (uint8_t i = 0; i < Player::hp; i++)
  {
    sprites.drawPlusMask(4 + i * 7, 0, ui_heart_plus_mask, 0);
  }

  // FIXME
//  uint16_t n;
//  if (Player::score >= 1000) n = 4;
//  else if (Player::score >= 100) n = 3;
//  else if (Player::score >= 10) n = 2;
//  else n = 1;
//  ab.setCursor(128 - n * 6, 0);
//  ab.print(Player::score);
  ab.fillRect(103, 0, 6 * FONT_PAD, 7, BLACK);
  Menu::drawNumber(104, 0, Game::score, 6);
}
