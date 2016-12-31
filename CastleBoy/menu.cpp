#include "menu.h"

#include "game.h"
#include "player.h"
#include "map.h"
#include "assets.h"

#define TITLE_OPTION_MAX 2
#define TITLE_OPTION_PLAY 0
#define TITLE_OPTION_HELP 1
#define TITLE_OPTION_SFX 2

namespace
{
uint8_t stage;
uint8_t counter;
bool flag;
bool toggle = 0;
uint8_t toggleSpeed;
uint8_t menuIndex;
int8_t titleLeftOffset;
int8_t titleRightOffset;
}

void Menu::showTitle()
{
  mainState = STATE_TITLE;
  flag = true;
  toggleSpeed = 20;
  counter = 60;
  menuIndex = TITLE_OPTION_PLAY;
  stage = 1;
  Game::reset();
}

void Menu::notifyPlayerDied()
{
  mainState = STATE_PLAYER_DIED;
  counter = 140;
  sound.tone(NOTE_G3, 100, NOTE_G2, 150, NOTE_G1, 350);
}

void Menu::notifyStageFinished()
{
  mainState = STATE_STAGE_FINISHED;
  counter = 80;
}

void showStageIntro()
{
  mainState = STATE_STAGE_INTRO;
  counter = 180;
  flashCounter = 2;
}

void drawMenuOption(uint8_t index, const uint8_t* sprite)
{
  uint8_t halfWidth = pgm_read_byte(sprite) / 2;
  sprites.drawOverwrite(64 - halfWidth, 40 + index * 8, sprite, 0);
  if (index == menuIndex)
  {
    sprites.drawOverwrite(55 - halfWidth, 38 + index * 8, entity_candle, toggle);
    sprites.drawOverwrite(68 + halfWidth, 38 + index * 8, entity_candle, toggle);
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

  if (ab.everyXFrames(20))
  {
    toggle = !toggle;
  }


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

    if (ab.justPressed(UP_BUTTON) && menuIndex > 0)
    {
      --menuIndex;
      sound.tone(NOTE_E6, 15);
    }

    if (ab.justPressed(DOWN_BUTTON) && menuIndex < TITLE_OPTION_MAX)
    {
      ++menuIndex;
      sound.tone(NOTE_E6, 15);
    }

    if (ab.justPressed(A_BUTTON))
    {
      switch (menuIndex)
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
          if (ab.audio.enabled())
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
    drawMenuOption(TITLE_OPTION_SFX, ab.audio.enabled() ? text_sfx_on : text_sfx_off);
  }
  sprites.drawOverwrite(36, 2 + titleLeftOffset, title_left, 0);
  sprites.drawOverwrite(69, 2 + titleRightOffset, title_right, 0);
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
      if (ab.everyXFrames(16))
      {
        toggle = !toggle;
      }
      sprites.drawOverwrite(52, 18, text_stage, 0);
      Util::drawNumber(75, 18, stage, ALIGN_LEFT);
      sprites.drawPlusMask(56, 32, player_plus_mask, toggle ? 2 : 0);
      if (--counter == 0)
      {
        Game::timeLeft = GAME_STARTING_TIME;
        flashCounter = 4;
        Game::play();
      }
      break;
    case STATE_GAME_OVER:
      if (ab.everyXFrames(16))
      {
        toggle = !toggle;
      }

      sprites.drawOverwrite(47, 0, text_game_over, 0);
      sprites.drawOverwrite(43, 16, game_over_head, 0);
      if (toggle)
      {
        sprites.drawOverwrite(58, 41, game_over_head_jaw, 0);
      }
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
    case STATE_STAGE_FINISHED:
      Game::loop();
      ab.fillRect(0, 22, 128, 20, BLACK);
      if (--counter == 0)
      {
        if (stage == STAGE_MAX)
        {
          mainState = STATE_GAME_FINISHED;
        }
        else
        {
          ++stage;
          showStageIntro();
        }
      }
      break;
    case STATE_PLAYER_DIED:
      Game::loop();

      if (--counter == 0)
      {
        if (Game::life == 0)
        {
          mainState = STATE_GAME_OVER;
        }
        else
        {
          Game::play();
        }

        flashCounter = 2;
      }

      if (counter > 100)
      {
        break;
      }
      
      ab.fillRect(0, 22, 128, 20, BLACK);
      if (Game::timeLeft == 0)
      {
        sprites.drawOverwrite(47, 29, text_time_up, 0);
      }
      else
      {
        sprites.drawOverwrite(52, 29, ui_life_count, 0);
        if (counter > 80)
        {
          Util::drawNumber(64, 29, Game::life + 1, ALIGN_LEFT);
        }
        else if (counter > 70)
        {
          Util::drawNumber(64, 28, Game::life, ALIGN_LEFT);
        }
        else
        {
          Util::drawNumber(64, 29, Game::life, ALIGN_LEFT);
        }
      }
      break;
  }
}

