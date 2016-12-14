#include "menu.h"

#include "game.h"
#include "player.h"
#include "assets.h"

#define TITLE_OPTION_MAX 2
#define TITLE_OPTION_PLAY 0
#define TITLE_OPTION_HELP 1
#define TITLE_OPTION_SFX 2

namespace
{
const uint8_t* const stages[] = { stage_1_1, stage_1_2, stage_1_3, stage_1_4, stage_2_1, stage_2_2, stage_2_3, stage_2_4, stage_3_1, stage_3_2, stage_3_3, stage_3_4 };

uint8_t counter;
bool flag;
bool toggle = 0;
uint8_t menuIndex;
int8_t titleLeftOffset;
int8_t titleRightOffset;
}

void Menu::showTitle()
{
  mainState = STATE_TITLE;
  flag = true;
  counter = 60;
  menuIndex = TITLE_OPTION_PLAY;

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
  counter = 120;
}

void drawMenuOption(uint8_t index, const uint8_t* sprite)
{
  uint8_t halfWidth = pgm_read_byte(sprite) / 2;
  sprites.drawOverwrite(64 - halfWidth, 40 + index * 8, sprite, 0);
  if(index == menuIndex)
  {
    sprites.drawOverwrite(53 - halfWidth, 38 + index * 8, entity_candle, toggle);
    sprites.drawOverwrite(66 + halfWidth, 38 + index * 8, entity_candle, toggle);
  }
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
      sound.tone(NOTE_GS3, 25, NOTE_G3, 15);
    }
  }
  else
  {
    if (ab.everyXFrames(80))
    {
      titleLeftOffset = titleLeftOffset == 0 ? 1 : 0;
      titleRightOffset = titleRightOffset == 0 ? 1 : 0;
    }    

    if(ab.justPressed(UP_BUTTON) && menuIndex > 0)
    {
      --menuIndex;
      sound.tone(NOTE_E6, 15);
    }

    if(ab.justPressed(DOWN_BUTTON) && menuIndex < TITLE_OPTION_MAX)
    {
      ++menuIndex;
      sound.tone(NOTE_E6, 15);
    }
    
    if (ab.justPressed(A_BUTTON))
    {      
      switch(menuIndex)
      {
        case TITLE_OPTION_PLAY:
          mainState = STATE_STAGE_INTRO;
          counter = 100;
          sound.tone(NOTE_CS6, 30);
          break;
        case TITLE_OPTION_HELP: 
          // TODO
          sound.tone(NOTE_CS6, 30);
          break;
        case TITLE_OPTION_SFX:
          if(ab.audio.enabled())
          {            
            ab.audio.off();
          }
          else
          {
            ab.audio.on();
          }
          ab.audio.saveOnOff();
          sound.tone(NOTE_CS6, 30);
          break;
      }      
    }

    drawMenuOption(TITLE_OPTION_PLAY, text_play);
    drawMenuOption(TITLE_OPTION_HELP, text_help);
    drawMenuOption(TITLE_OPTION_SFX, ab.audio.enabled() ? text_sfx_off : text_sfx_on);
  }
  sprites.drawOverwrite(36, 5 + titleLeftOffset, title_left, 0);
  sprites.drawOverwrite(69, 5 + titleRightOffset, title_right, 0);  
}

void Menu::loop()
{
  if(ab.everyXFrames(20))
  {
    toggle = !toggle;
  }
  
  switch (mainState)
  {
    case STATE_TITLE:
      loopTitle();
      break;
    case STATE_PLAY:
      Game::loop();
      break;
    case STATE_STAGE_INTRO:
      sprites.drawOverwrite(46, 22, text_stage, 0);
      Util::drawNumber(69, 22, Game::stageIndex / 3 + 1, ALIGN_LEFT);
      Util::drawNumber(76, 22, Game::stageIndex % 3 + 1, ALIGN_LEFT);
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
      if(Game::timeLeft == 0)
      {
        sprites.drawOverwrite(47, 27, text_time_up, 0);
      }
      else
      {
        sprites.drawOverwrite(47, 27, text_game_over, 0);
      }
      sprites.drawOverwrite(60, 40, entity_skull, toggle);
      if (ab.justPressed(A_BUTTON))
      {
        Menu::showTitle();
      }
      break;
    case STATE_GAME_FINISHED:
      sprites.drawOverwrite(42, 22, text_final_score, 0);
      Util::drawNumber(64, 38, Game::timeLeft / FPS, ALIGN_CENTER);
      if (ab.justPressed(A_BUTTON))
      {
        Menu::showTitle();
      }
      break;
  }
}

