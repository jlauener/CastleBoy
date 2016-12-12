#include "game.h"

#include "assets.h"
#include "map.h"
#include "player.h"
#include "entity.h"
#include "menu.h"

namespace
{
uint8_t deathCounter = 0;
bool restoreHp = false;
}

int16_t Game::cameraX;

void Game::play(const uint8_t* source)
{
  mainState = STATE_PLAY;
  if (restoreHp)
  {
    Player::hp = PLAYER_MAX_HP;
    restoreHp = false;
  }
  Entities::init();
  Map::init(source);
  cameraX = 0;
}

void Game::loop()
{
  // debug
  #ifdef DEBUG_CHEAT
    if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
    {
      Menu::onStageFinished();
      return;
    }
  #endif

  // update
  Player::update();
  Entities::update();

  // check if stage is finished
  if (Player::pos.x - 4 /*normalHitbox.x*/ > Map::width * TILE_WIDTH)
  {
    Menu::onStageFinished();
  }

  // check if player is dead
  if (deathCounter == 0 && !Player::alive)
  {
    deathCounter = 100;
    restoreHp = true;
    //Menu::playMusic();
    sound.tone(NOTE_G3, 100, NOTE_G2, 150, NOTE_G1, 350);
  }

  if (deathCounter > 0)
  {
    if (--deathCounter == 0)
    {
      Menu::onPlayerDie();
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

  // draw: parralax
  if ((Menu::stageIndex == 0 || Menu::stageIndex == 2) /* FIXME */)
  {
    int16_t backgroundOffset = cameraX / 28; // FIXME properly calculate parralax unless all maps have same width
    sprites.drawOverwrite(16 - backgroundOffset, 4, background_mountain, 0);
  }

  // draw: main
  Map::draw();
  Entities::draw();
  Player::draw();

  // ui: hp
  ab.fillRect(0, 0, 4 * PLAYER_MAX_HP, 7, BLACK);
  for (uint8_t i = 0; i < PLAYER_MAX_HP; i++)
  {
    sprites.drawSelfMasked(i * 4, 0, i < Player::hp ? ui_hp_full : ui_hp_empty, 0);
  }

  // ui: knife count
  ab.fillRect(52, 0, 24, 7, BLACK);
  sprites.drawSelfMasked(52, 0, ui_knife_count, 0);  
  Util::drawNumber(64, 0, Player::knifeCount, 2);

  // ui: time left
  ab.fillRect(103, 0, 1 + 6 * FONT_PAD, 7, BLACK);
  Util::drawNumber(104, 0, Menu::timeLeft, 6);
}
