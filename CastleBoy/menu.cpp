#include "menu.h"

#include "game.h"
#include "player.h"
#include "assets.h"

namespace
{
const uint8_t* const stages[] = { stage_1, stage_2, stage_3, stage_4 };

uint8_t counter;
bool flag;
int8_t titleLeftOffset;
int8_t titleRightOffset;
}

void Menu::showTitle()
{
  mainState = STATE_TITLE;
  flag = true;
  counter = 60;

  // reset game
  Game::life = GAME_STARTING_LIFE;
  Game::stageIndex = 0;
  Game::timeLeft = GAME_STARTING_TIME;
  Player::hp = PLAYER_MAX_HP;
  Player::knifeCount = 0;
}

void Menu::showStageIntro()
{
  mainState = STATE_STAGE_INTRO;
  counter = 100;
}

void loopTitle()
{
#ifdef DEBUG_CHEAT
  if (ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
  {
    Menu::showTitle();
    return;
  }
#endif

  if (flag)
  {
    titleLeftOffset = counter * 2;
    titleRightOffset = -counter * 2;

    if (--counter == 0)
    {
      titleLeftOffset = 1;
      titleRightOffset = 0;
      flag = false;
      flashCounter = 6;
    }
  }
  else
  {
    if (ab.everyXFrames(80))
    {
      titleLeftOffset = titleLeftOffset == 0 ? 1 : 0;
      titleRightOffset = titleRightOffset == 0 ? 1 : 0;
    }

    if (ab.justPressed(A_BUTTON))
    {
      sound.tone(NOTE_CS6, 30);
      mainState = STATE_STAGE_INTRO;
      counter = 100;
    }
  }
  sprites.drawOverwrite(36, 5 + titleLeftOffset, title_left, 0);
  sprites.drawOverwrite(69, 5 + titleRightOffset, title_right, 0);
}

void Menu::loop()
{
  switch (mainState)
  {
    case STATE_TITLE:
      loopTitle();
      break;
    case STATE_PLAY:
      Game::loop();
      break;
    case STATE_STAGE_INTRO:
      sprites.drawOverwrite(51, 22, text_stage, 0);
      Util::drawNumber(75, 22, Game::stageIndex + 1, ALIGN_LEFT);
      if(Game::hasPlayerDied)
      {
        sprites.drawOverwrite(52, 38, ui_life_count, 0);
        Util::drawNumber(64, 38, Game::life, ALIGN_LEFT);
      }
      if (--counter == 0)
      {
        Game::play(stages[Game::stageIndex]);
      }
      break;
    case STATE_GAME_OVER:
      sprites.drawOverwrite(47, 27, text_game_over, 0);
      if (ab.justPressed(A_BUTTON))
      {
        Menu::showTitle();
      }
      break;
    case STATE_GAME_FINISHED:
      sprites.drawOverwrite(42, 27, text_final_score, 0);
      Util::drawNumber(52, 39, Game::timeLeft / FPS, ALIGN_CENTER);
      if (ab.justPressed(A_BUTTON))
      {
        Menu::showTitle();
      }
      break;
  }
}

