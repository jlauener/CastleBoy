#include "menu.h"

#include "game.h"
#include "assets.h"
#include "sounds.h"
#include "player.h"

uint8_t Menu::life;
uint16_t Menu::timeLeft;
uint8_t Menu::stageIndex;

namespace
{
#define MAX_STAGE 4
const uint8_t* const stages[] = { stage_1, stage_2, stage_3, stage_4 };

uint8_t counter;
bool titleIntro;
int8_t titleLeftOffset;
int8_t titleRightOffset;
}

//const uint16_t* music = NULL;
//uint16_t musicCounter = 0;
//uint8_t musicIndex = 0;

//void Menu::playMusic(const uint16_t* music_)
//{
//  music = music_;
//  musicCounter = 1;
//  musicIndex = 0;
//}
//
//void Menu::updateMusic()
//{
//  if(music == NULL)
//  {
//    return;
//  }
//
//  if(--musicCounter == 0)
//  {
//    int16_t newTone = music[musicIndex++];
//    if(newTone == TONES_REPEAT)
//    {
//      newTone = music[0];
//      musicIndex = 1;
//    }
//
//    musicCounter =  music[musicIndex++] / 16; // duration
//    if(newTone != 0)
//    {
//      sound.tone(newTone, musicCounter * 10);
//    }
//  }
//}

void Menu::showTitle()
{
  mainState = STATE_TITLE;
  titleIntro = true;
  counter = 60;

  // reset game
  life = PLAYER_STARTING_LIFE;
  stageIndex = 0;
  timeLeft = PLAYER_STARTING_TIME;
  Player::hp = PLAYER_MAX_HP;
  Player::knifeCount = 0;
}

void Menu::onStageFinished()
{
  //playMusic();
  if (++stageIndex == MAX_STAGE)
  {
    mainState = STATE_GAME_FINISHED;
  }
  else
  {
    mainState = STATE_STAGE_INTRO;
    counter = 100;
  }
}

void Menu::onPlayerDie()
{
  Player::knifeCount = 0;
  if (--life == 0)
  {
    mainState = STATE_GAME_OVER;
  }
  else
  {
    mainState = STATE_STAGE_INTRO;
    counter = 60;
  }
}

void updateTitle()
{
#ifdef DEBUG_CHEAT
  if (ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
  {
    Menu::showTitle();
    return;
  }
#endif

  if (titleIntro)
  {
    titleLeftOffset = counter * 2;
    titleRightOffset = -counter * 2;

    if (--counter == 0)
    {
      titleLeftOffset = 1;
      titleRightOffset = 0;
      //sound.tones(beat2);
      titleIntro = false;
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
      sound.tones(sfx_select);
      mainState = STATE_STAGE_INTRO;
      counter = 100;
    }
  }
  sprites.drawOverwrite(36, 5 + titleLeftOffset, title_left, 0);
  sprites.drawOverwrite(69, 5 + titleRightOffset, title_right, 0);
}

void updateStageIntro()
{
  sprites.drawOverwrite(51, 22, text_stage, 0);
  Util::drawNumber(75, 22, Menu::stageIndex + 1);
  sprites.drawOverwrite(54, 38, ui_life_count, 0);
  Util::drawNumber(64, 38, Menu::life);
  if (--counter == 0)
  {
    //playMusic(music1);
    Game::play(stages[Menu::stageIndex]);
  }
}

void updateGameOver()
{
  sprites.drawOverwrite(47, 27, text_game_over, 0);
  if (ab.justPressed(A_BUTTON))
  {
    Menu::showTitle();
  }
}

void updateGameFinished()
{
  sprites.drawOverwrite(42, 27, text_final_score, 0);
  Util::drawNumber(52, 39, Menu::timeLeft, 6);
  if (ab.justPressed(A_BUTTON))
  {
    Menu::showTitle();
  }
}

void Menu::loop()
{
  //updateMusic();

  switch (mainState)
  {
    case STATE_TITLE:
      updateTitle();
      break;
    case STATE_PLAY:
      Game::loop();
      break;
    case STATE_STAGE_INTRO:
      updateStageIntro();
      break;
    case STATE_GAME_OVER:
      updateGameOver();
      break;
    case STATE_GAME_FINISHED:
      updateGameFinished();
      break;
  }
}

