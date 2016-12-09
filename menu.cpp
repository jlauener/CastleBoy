#include "menu.h"

#include "game.h"
#include "assets.h"
#include "player.h"

uint8_t Menu::life;
uint16_t Menu::score;
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

void Menu::showTitle()
{
  mainState = STATE_TITLE;
  titleIntro = true;
  counter = 60;

  // reset game
  life = STARTING_LIFE;
  stageIndex = 0;
  score = 0;
  Player::hp = STARTING_HP;
}

void Menu::onStageFinished()
{
  if (++stageIndex == MAX_STAGE)
  {
    mainState = STATE_GAME_OVER;
  }
  else
  {
    mainState = STATE_STAGE_INTRO;
    counter = 100;
  }
}

void Menu::onPlayerDie()
{
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

void Menu::loop()
{
  switch (mainState)
  {
    case STATE_TITLE:
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
          sound.tone(NOTE_CS6, 30, NOTE_CS5, 40);
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
          mainState = STATE_STAGE_INTRO;
          counter = 100;
        }
      }
      sprites.drawOverwrite(36, 5 + titleLeftOffset, title_left, 0);
      sprites.drawOverwrite(69, 5 + titleRightOffset, title_right, 0);
      break;
    case STATE_STAGE_INTRO:
      sprites.drawOverwrite(51, 29, text_stage, 0);
      drawNumber(75, 29, stageIndex + 1);
      if (--counter == 0)
      {
        Game::play(stages[stageIndex]);
      }
      break;
    case STATE_GAME_OVER:
      sprites.drawOverwrite(42, 27, text_final_score, 0);
      drawNumber(52, 39, score, 6);
      if (ab.justPressed(A_BUTTON))
      {
        showTitle();
      }
      break;
  }
}

// Inspired by TEAMArg's Sirene, stages.h:775
// But optimized (use of int8_t, use cast instead of for loop)
void Menu::drawNumber(int16_t x, int16_t y, uint16_t value, uint8_t zeroPad)
{
  char buf[10];
  ltoa(value, buf, 10);
  uint8_t strLength = strlen(buf);
  uint8_t pad = zeroPad > strLength ? zeroPad - strLength : 0;

  // draw 0 padding
  for (uint8_t i = 0; i < pad; i++)
  {
    sprites.drawSelfMasked(x + FONT_PAD * i, y, font, 0);
  }

  // draw the number
  for (uint8_t i = 0; i < strLength; i++)
  {
    uint8_t digit = (uint8_t) buf[i];
    digit -= 48;
    if (digit > 9) digit = 0;
    sprites.drawSelfMasked(x + pad * FONT_PAD + FONT_PAD * i, y, font, digit);
  }
}

