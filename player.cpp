#include "player.h"

#include "entity.h"
#include "data.h"
#include "map.h"
#include "candle.h"
#include "assets.h"

namespace
{
Entity p;
int16_t velocityX;
int16_t velocityY;
// FIXME move flags to a single byte and use masking (is it worth it?)
bool grounded;
bool attacking;
bool jumping;
bool ducking;
int8_t levitateCounter;

const uint8_t idleAnim[] = {LOOP, /* FRAME_RATE */ 0, /* FRAME_COUNT */ 1, /* FRAMES */ 0};
const uint8_t walkAnim[] = {LOOP, /* FRAME_RATE */ 12, /* FRAME_COUNT */ 2, /* FRAMES */ 0, 1};
const uint8_t attackAnim[] = {ONE_SHOT, /* FRAME_RATE */ 10, /* FRAME_COUNT */ 2, /* FRAMES */ 2, 3, 3};
const uint8_t airAnim[] = {LOOP, /* FRAME_RATE */ 0, /* FRAME_COUNT */ 1, /* FRAMES */ 4};
const uint8_t duckIdleAnim[] = {LOOP, /* FRAME_RATE */ 0, /* FRAME_COUNT */ 1, /* FRAMES */ 5};
const uint8_t duckAttackAnim[] = {ONE_SHOT, /* FRAME_RATE */ 10, /* FRAME_COUNT */ 2, /* FRAMES */ 6, 7, 7};

} // unamed

void Player::init(int16_t x, int16_t y)
{
  p.init(Data::player, x, y);
  grounded = false;
  attacking = false;
  jumping = false;
  ducking = false;
  levitateCounter = 0;
  velocityX = 0;
  velocityY = 0;
  p.play(idleAnim);
}

void Player::update()
{
  // duck
  ducking = grounded && ab.pressed(DOWN_BUTTON);

  // attack
  if (!attacking && ab.justPressed(B_BUTTON))
  {
    attacking = true;
    if (ducking)
    {
      p.play(duckAttackAnim);
    }
    else
    {
      p.play(attackAnim);
    }
  }

  if (attacking)
  {
    if (!p.isPlaying())
    {
      attacking = false;
    }
  }

  // vertical movement
  if (grounded && ab.justPressed(A_BUTTON))
  {
    // start jumping
    grounded = false;
    jumping = true;
    velocityY = -PLAYER_JUMP_FORCE_F;
  }

  if (levitateCounter > 0)
  {
    --levitateCounter;
  }
  else if (jumping)
  {
    velocityY += PLAYER_JUMP_GRAVITY_F;
    if (velocityY >= 0)
    {
      velocityY = 0;
      jumping = false;
      levitateCounter = PLAYER_JUMP_LEVITATE;
    }
    else
    {
      p.moveY(velocityY / F_PRECISION);
    }
  }
  else
  {
    velocityY += PLAYER_FALL_GRAVITY_F;
    int16_t offsetY = velocityY / F_PRECISION;
    if (offsetY > 0)
    {
      grounded = p.moveY(offsetY);
    }
    else
    {
      grounded = p.collideWithMap(p.x, p.y + 1);
    }

    if (grounded)
    {
      velocityY = 0;
    }
  }

  //LOG_DEBUG(grounded ? "GROUNDED" : "AIR");

  // horizontal movement
  if (!attacking && !ducking && ab.pressed(LEFT_BUTTON))
  {
    velocityX = -1;
    p.flipped = true;
  }
  else if (!attacking && !ducking && ab.pressed(RIGHT_BUTTON))
  {
    velocityX = 1;
    p.flipped = false;
  }
  else if (grounded)
  {
    velocityX = 0;
  }

  if (ab.everyXFrames(2) && velocityX != 0)
  {
    p.moveX(velocityX);
  }

  // update camera, if needed
  if(p.x < cameraX + CAMERA_BUFFER)
  {
    cameraX = p.x - CAMERA_BUFFER;
    if(cameraX < 0) cameraX = 0;
  }
  else if(p.x > cameraX + 128 - CAMERA_BUFFER)
  {
    cameraX = p.x - 128 + CAMERA_BUFFER;
    if(cameraX > Map::width() * TILE_WIDTH - 128) cameraX = Map::width() * TILE_WIDTH - 128;
  }
}

void Player::draw()
{
  if (!attacking)
  {
    if (grounded)
    {
      if (ducking)
      {
        p.play(duckIdleAnim);
        LOG_DEBUG("DUCK");
      }
      else
      {
        p.play(velocityX != 0 ? walkAnim : idleAnim);
        LOG_DEBUG(velocityX != 0 ? "WALK" : "IDLE");
      }
    }
    else
    {
      p.play(airAnim);
      LOG_DEBUG("AIR");
    }
  }
  else
  {
    LOG_DEBUG(ducking ? "DUCK ATTACK" : "ATTACK");
  }

  p.draw();

  if(attacking)
  {
    if(p.animFrame > 0)
    {
      ab.drawFastHLine(p.x + (p.flipped ? -24 : 8) - cameraX , p.y - (ducking ? 4 : 8), 16);
      // BAD BAD BAD
      Candles::hit(p.x + (p.flipped ? -24 : 8), p.y - (ducking ? 4 : 8), 16);
    }
  }
}

