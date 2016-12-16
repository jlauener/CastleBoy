#include "game.h"

#include "menu.h"
#include "map.h"
#include "player.h"
#include "entity.h"
#include "assets.h"

int16_t Game::cameraX;
uint8_t Game::life;
uint16_t Game::timeLeft;
uint8_t Game::stageIndex;
bool Game::hasPlayerDied = false;

namespace
{
uint8_t deathCounter = 0;
uint8_t finishedCounter = 0;

void drawHpBar(int16_t x, int16_t y, uint8_t value, uint8_t max)
{
  ab.fillRect(x, y, 4 * max, 7, BLACK);
  for (uint8_t i = 0; i < max; i++)
  {
    sprites.drawSelfMasked(x + i * 4, y, i < value ? ui_hp_full : ui_hp_empty, 0);
  }
}

}

void Game::play(const uint8_t* source)
{
  mainState = STATE_PLAY;
  if (hasPlayerDied)
  {
    Player::hp = PLAYER_MAX_HP;
    hasPlayerDied = false;
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
    Menu::showStageIntro();
    return;
  }

  if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(UP_BUTTON))
  {
    if (++stageIndex == STAGE_MAX)
    {
      mainState = STATE_GAME_FINISHED;
    }
    else
    {
      Menu::showStageIntro();
    }
    return;
  }

  if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(LEFT_BUTTON))
  {
    play(stage_test);
  }
#endif

  // update
  Player::update();
  Entities::update();

  if (timeLeft > 0)
  {
    --timeLeft;
  }

  // finished: exit from left
  if (finishedCounter == 0 && Player::pos.x - 4 /*normalHitbox.x*/ > Map::width * TILE_WIDTH)
  {
    finishedCounter = 1;
  }

  // finished: boss killed
  if (finishedCounter == 0 && Map::boss != NULL && Map::boss->hp == 0)
  {
    finishedCounter = 120;
  }

  // finished: update counter
  if (finishedCounter > 0)
  {
    if (--finishedCounter == 0)
    {
      if (++stageIndex == STAGE_MAX)
      {
        mainState = STATE_GAME_FINISHED;
      }
      else
      {
        Menu::showStageIntro();
      }
    }
  }

  // game over: check if player is alive
  if (deathCounter == 0 && (!Player::alive || timeLeft == 0))
  {
    Player::knifeCount = 0;
    if (timeLeft == 0)
    {
      Game::life = 0;
    }
    else
    {
      --Game::life;
    }

    deathCounter = 160;
    //Menu::playMusic();
    sound.tone(NOTE_G3, 100, NOTE_G2, 150, NOTE_G1, 350);
  }

  // game over: update counter
  if (deathCounter > 0)
  {
    if (--deathCounter == 0)
    {
      if (life == 0)
      {
        mainState = STATE_GAME_OVER;
      }
      else
      {
        hasPlayerDied = true;
        Menu::showStageIntro();
      }
    }
  }

  // update camera
  if (Player::pos.x < cameraX + CAMERA_LEFT_BUFFER)
  {
    cameraX = Player::pos.x - CAMERA_LEFT_BUFFER;
    if (cameraX < 0) cameraX = 0;
  }
  else if (Player::pos.x > cameraX + 128 - CAMERA_RIGHT_BUFFER)
  {
    cameraX = Player::pos.x - 128 + CAMERA_RIGHT_BUFFER;
    if (cameraX > Map::width * TILE_WIDTH - 128) cameraX = Map::width * TILE_WIDTH - 128;
  }

  // draw: parralax
  if (Map::showBackground)
  {
    int16_t backgroundOffset = cameraX / 28; // FIXME properly calculate parralax unless all maps have same width
    sprites.drawOverwrite(16 - backgroundOffset, 4, background_mountain, 0);
  }

  // draw: main
  Map::draw();
  Entities::draw();
  Player::draw();

  // ui: hp
  drawHpBar(0, 0, Player::hp, PLAYER_MAX_HP);

  // ui: knife count
  ab.fillRect(54, 0, 13, 7, BLACK);
  sprites.drawSelfMasked(55, 0, ui_knife_count, 0);

  Util::drawNumber(68, 0, Player::knifeCount, ALIGN_LEFT);

  // ui: time left
  Util::drawNumber(128, 0, timeLeft / FPS, ALIGN_RIGHT);

  // ui: boss
  if (Map::boss != NULL)
  {
    drawHpBar(40, 58, Map::boss->hp, BOSS_MAX_HP);
  }
}

bool Game::moveY(Vec& pos, int8_t dy, const Box& hitbox, bool collideToEntity)
{
  if (dy != 0)
  {
    int8_t sign = dy > 0 ? 1 : -1;
    while (dy != 0)
    {
      if (Map::collide(pos.x, pos.y + sign, hitbox) || (collideToEntity && Entities::moveCollide(pos.x, pos.y + sign, hitbox)))
      {
        return true;
      }
      pos.y += sign;
      dy -= sign;
    }
  }

  return false;
}
