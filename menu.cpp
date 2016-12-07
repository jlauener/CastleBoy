#include "menu.h"

#include "game.h"
#include "assets.h"

namespace
{
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
}

void Menu::showStageIntro()
{
  mainState = STATE_STAGE_INTRO;
  counter = 100;
}

void Menu::showGameOver()
{
  mainState = STATE_GAME_OVER;
}

void Menu::loop()
{
  switch (mainState)
  {
    case STATE_TITLE:
      if (ab.pressed(A_BUTTON) && ab.pressed(B_BUTTON) && ab.pressed(DOWN_BUTTON))
      {
        Menu::showTitle();
        return;
      }

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
          Game::reset();
          showStageIntro();
        }
      }
      sprites.drawOverwrite(36, 5 + titleLeftOffset, title_left, 0);
      sprites.drawOverwrite(69, 5 + titleRightOffset, title_right, 0);
      break;
    case STATE_STAGE_INTRO:
      ab.setCursor(4, 4);
      ab.print("STAGE 1-1");
      if (--counter == 0)
      {
        Game::play();
      }
      break;
    case STATE_GAME_OVER:
      ab.setCursor(4, 4);
      ab.print("FINAL SCORE");
      drawNumber(52, 34, Game::score);
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

