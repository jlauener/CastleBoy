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
uint8_t menuIndex;
int8_t titleOffset;
//int8_t titleRightOffset;
}

void Menu::showTitle()
{
  mainState = STATE_TITLE;
  flag = true;
  counter = 60;
  menuIndex = TITLE_OPTION_PLAY;
  stage = 1;
  Player::hp = PLAYER_MAX_HP;
  Game::reset();
}

void Menu::notifyPlayerDied()
{
  mainState = STATE_PLAYER_DIED;
  counter = 140;
  sound.tone(NOTE_G3, 100, NOTE_G2, 150, NOTE_G1, 350);
}

void Menu::notifyLevelFinished()
{
  if (Map::boss != NULL)
  {
    mainState = STATE_STAGE_FINISHED;
    counter = 80;
    sound.tone(NOTE_G2, 100, NOTE_G3, 150, NOTE_G4, 350);
  }
  else
  {
    mainState = STATE_LEVEL_FINISHED;
    counter = 40;
  }
}

void showStageIntro()
{
  mainState = STATE_STAGE_INTRO;
  counter = 180;
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
    titleOffset = counter * 2;
    //titleRightOffset = -counter * 2;

    if (--counter == 0)
    {
      titleOffset = 1;
      //titleRightOffset = 0;
      flag = false;
      flashCounter = 6;
      sound.tone(NOTE_GS3, 25, NOTE_G3, 15);
    }
  }
  else
  {
    if (ab.everyXFrames(80))
    {
      //titleLeftOffset = titleLeftOffset == 0 ? 1 : 0;
      //titleRightOffset = titleRightOffset == 0 ? 1 : 0;
      titleOffset = -titleOffset;
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
          mainState = STATE_HELP;
          counter = 32;
          flag = false;
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
  sprites.drawOverwrite(36, 2 - titleOffset, title_left, 0);
  sprites.drawOverwrite(69, 2 + titleOffset, title_right, 0);
}

void loopHelp()
{
  if (ab.justPressed(A_BUTTON))
  {
    Menu::showTitle();
    sound.tone(NOTE_E6, 15);
  }
}

void loopEnd(bool won)
{
  if (ab.everyXFrames(4))
  {
    if (--counter == 0)
    {
      if (!flag)
      {
        sound.tone(NOTE_CS6, 30, NOTE_CS7, 40);
        flashCounter = 6;
        flag = true;
      }
      counter = 40;
    }
  }

  bool shift = (won && flag) ? counter < 20 : 0;
  uint8_t offset = flag ? 0 : counter;

  sprites.drawOverwrite(2, 48 + offset / 2, background_mountain, 0);
  sprites.drawOverwrite(0, 44 + offset, end_hill, 0);

  if (won)
  {
    sprites.drawOverwrite(16, 28 + offset, end_player, shift);
    sprites.drawOverwrite(50 - shift, 8 - offset, text_the_end, 0);
  }
  else
  {
    sprites.drawOverwrite(20, 36 + offset, tileset, 8);
    sprites.drawOverwrite(47, 8 - offset, text_game_over, 0);
  }

  if (flag)
  {
    sprites.drawOverwrite(54 + shift, 26, text_score, 0);
    Util::drawNumber(64 + shift, 34, Game::score, ALIGN_CENTER);
  }

  if (ab.justPressed(A_BUTTON))
  {
    Menu::showTitle();
    sound.tone(NOTE_E6, 15);
  }
}

void Menu::loop()
{
  switch (mainState)
  {
    case STATE_TITLE:
      loopTitle();
      break;
    case STATE_HELP:
      loopHelp();
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
        Game::play();
      }
      break;
    case STATE_GAME_OVER:
      loopEnd(false);
      break;
    case STATE_GAME_FINISHED:
      loopEnd(true);
      break;
    case STATE_LEVEL_FINISHED:
      if (--counter == 0)
      {
        Game::play();
      }
      break;
    case STATE_STAGE_FINISHED:
      if (Game::timeLeft > 0)
      {
        if (counter > 0)
        {
          --counter;
        }
        else
        {
          Game::score += SCORE_PER_SECOND;
          if (Game::timeLeft > FPS)
          {
            Game::timeLeft -= FPS;
          }
          else
          {
            Game::timeLeft = 0;
            counter = 60;
          }

          if (ab.everyXFrames(8))
          {
            sound.tone(NOTE_G4, 25);
          }
        }
      }
      else if (Player::hp < PLAYER_MAX_HP)
      {
        if (--counter == 0)
        {
          ++Player::hp;
          sound.tone(NOTE_G3, 60);
          counter = Player::hp == PLAYER_MAX_HP ? 90 : 20;
        }
      }
      else if (--counter == 0)
      {
        if (stage == STAGE_MAX)
        {
          mainState = STATE_GAME_FINISHED;
          counter = 32;
          flag = false;
        }
        else
        {
          ++stage;
          showStageIntro();
        }
      }

      Game::loop();
      // TODO text 'stage cleared' ?
      if (Game::timeLeft > 0)
      {
        ab.fillRect(0, 21, 128, 22, BLACK);
        sprites.drawOverwrite(54, 23, text_score, 0);
        Util::drawNumber(64, 35, Game::score, ALIGN_CENTER);
      }
      break;
    case STATE_PLAYER_DIED:
      if (--counter == 0)
      {
        if (Game::life == 0)
        {
          mainState = STATE_GAME_OVER;
          counter = 32;
          flag = false;
        }
        else
        {
          Player::hp = PLAYER_MAX_HP;
          Game::play();
        }
      }
      else if (counter < 100)
      {

        if (Game::timeLeft == 0)
        {
          sprites.drawOverwrite(47, 29, text_time_up, 0);
        }
        else
        {
          sprites.drawOverwrite(57, 29, ui_life_count, 0);
          if (counter > 80)
          {
            Util::drawNumber(69, 29, Game::life + 1, ALIGN_LEFT);
          }
          else if (counter > 70)
          {
            Util::drawNumber(69, 28, Game::life, ALIGN_LEFT);
          }
          else
          {
            Util::drawNumber(69, 29, Game::life, ALIGN_LEFT);
          }

          if (counter == 80)
          {
            sound.tone(NOTE_GS3, 15);
          }
        }
      }
      else
      {
        Game::loop();
      }
      break;
  }
}

