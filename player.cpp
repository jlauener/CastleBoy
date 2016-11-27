#include "player.h"

#include "map.h"
#include "candle.h"
#include "assets.h"

// FIXME Cannot fall in a single tile

#define ATTACK_TOTAL_DURATION 20
#define ATTACK_CHARGE 16

#define SPRITE_ORIGIN_X 8
#define SPRITE_ORIGIN_Y 16

#define FRAME_IDLE 0
#define FRAME_WALK_1 0
#define FRAME_WALK_2 1
#define FRAME_ATTACK_CHARGE 2
#define FRAME_ATTACK 3
#define FRAME_AIR 4
#define FRAME_IDLE_DUCK 5
#define FRAME_WALK_DUCK_1 5
#define FRAME_WALK_DUCK_2 5
#define FRAME_ATTACK_CHARGE_DUCK 6
#define FRAME_ATTACK_DUCK 7

#define FRAME_FLIPPED_OFFSET 8

#define WALK_FRAME_RATE 12

namespace
{
int16_t playerX;
int16_t playerY;
int16_t velocityX;
int16_t velocityY;
// FIXME move flags to a single byte and use masking (is it worth it?)
bool grounded;
uint8_t attackCounter;
bool attacking;
bool jumping;
bool ducking;
int8_t levitateCounter;
bool flipped;
uint8_t walkFrameCounter;
bool walkFrame;

Rect normalHitbox;
Rect duckHitbox;

bool moveX(int16_t dx, const Rect& hitbox)
{
  if (dx != 0)
  {
    int8_t sign = dx > 0 ? 1 : -1;
    while (dx != 0)
    {
      if (Map::collide(playerX + sign, playerY, hitbox))
      {
        return true;
      }
      playerX += sign;
      dx -= sign;
    }
  }

  return false;
}

bool moveY(int16_t dy, const Rect& hitbox)
{
  if (dy != 0)
  {
    int8_t sign = dy > 0 ? 1 : -1;
    while (dy != 0)
    {
      if (Map::collide(playerX, playerY + sign, hitbox))
      {
        return true;
      }
      playerY += sign;
      dy -= sign;
    }
  }

  return false;
}

//void play(const uint8_t* anim_)
//{
//  if (anim != anim_ || anim[MODE] == ONE_SHOT)
//  {
//    anim = anim_;
//    animFrame = 0;
//    animCounter = anim[FRAME_RATE];
//  }
//}

} // unamed

void Player::init(int16_t x, int16_t y)
{
  // FIXME init only once
  normalHitbox.width = 8;
  normalHitbox.height = 14;
  normalHitbox.x = 4;
  normalHitbox.y = 14;

  duckHitbox.width = 8;
  duckHitbox.height = 6;
  duckHitbox.x = 4;
  duckHitbox.y = 6;
  ///

  playerX = x;
  playerY = y;
  grounded = false;
  attackCounter = 0;
  attacking = false;
  jumping = false;
  ducking = false;
  levitateCounter = 0;
  velocityX = 0;
  velocityY = 0;
}

void Player::update()
{
  // attack
  if (attackCounter == 0 && ab.justPressed(B_BUTTON))
  {
    attackCounter = ATTACK_TOTAL_DURATION;
  }

  if (attackCounter > 0)
  {
    --attackCounter;
  }

  // jump
  if (attackCounter == 0 && grounded && ab.justPressed(A_BUTTON))
  {
    // start jumping
    grounded = false;
    jumping = true;
    velocityY = -PLAYER_JUMP_FORCE_F;
  }

  // vertical movement
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
      moveY(velocityY / F_PRECISION, ducking ? duckHitbox : normalHitbox);
    }
  }
  else
  {
    velocityY += PLAYER_FALL_GRAVITY_F;
    int16_t offsetY = velocityY / F_PRECISION;
    if (offsetY > 0)
    {
      grounded = moveY(offsetY, ducking ? duckHitbox : normalHitbox);
    }
    else
    {
      grounded = Map::collide(playerX, playerY + 1, ducking ? duckHitbox : normalHitbox);
    }

    if (grounded)
    {
      velocityY = 0;
    }
  }

  // horizontal movement
  if (attackCounter == 0 && ab.pressed(LEFT_BUTTON))
  {
    velocityX = -1;
    flipped = true;
  }
  else if (attackCounter == 0 && ab.pressed(RIGHT_BUTTON))
  {
    velocityX = 1;
    flipped = false;
  }
  else if (grounded)
  {
    velocityX = 0;
  }

  // FIXME is it good to use every X frame? when jumpin it can make 3 tile jump fail maybe
  // --> simply make is so that we have few pixels margin.. ?
  if (ab.everyXFrames(ducking ? 4 : 2) && velocityX != 0)
  {
    moveX(velocityX, ducking ? duckHitbox : normalHitbox);
  }

  // duck
  if (attackCounter == 0)
  {
    if(!ducking)
    {
       ducking = grounded && ab.pressed(DOWN_BUTTON);
    }
    else if(!ab.pressed(DOWN_BUTTON))
    {
      // only stop ducking if player can stand
      ducking = Map::collide(playerX, playerY, normalHitbox);
    }
  }

  // perform attack
  if (attackCounter != 0 && attackCounter < ATTACK_CHARGE)
  {
    //ab.drawFastHLine(playerX + (flipped ? -24 : 8) - cameraX , playerY - (ducking ? 3 : 11), 16);
    //Candles::hit(playerX + (flipped ? -20 : 4), playerY - (ducking ? 3 : 11), 20);
    Candles::hit(playerX + (flipped ? -28 : 0), playerY - (ducking ? 3 : 11), 28);
  }

  // update camera, if needed
  if (playerX < cameraX + CAMERA_BUFFER)
  {
    cameraX = playerX - CAMERA_BUFFER;
    if (cameraX < 0) cameraX = 0;
  }
  else if (playerX > cameraX + 128 - CAMERA_BUFFER)
  {
    cameraX = playerX - 128 + CAMERA_BUFFER;
    if (cameraX > Map::width() * TILE_WIDTH - 128) cameraX = Map::width() * TILE_WIDTH - 128;
  }

}

void Player::draw()
{
  uint8_t frame = 0;

  if (attackCounter == 0)
  {
    if (grounded)
    {
      if (velocityX == 0)
      {
        frame = ducking ? FRAME_IDLE_DUCK : FRAME_IDLE;
        LOG_DEBUG("IDLE");
      }
      else
      {
        if (ab.everyXFrames(WALK_FRAME_RATE))
        {
          walkFrame = !walkFrame;
        }
        frame = ducking ? (walkFrame ? FRAME_WALK_DUCK_2 : FRAME_WALK_DUCK_1) : (walkFrame ? FRAME_WALK_2 : FRAME_WALK_1);
        LOG_DEBUG("WALK");
      }
    }
    else
    {
      frame = FRAME_AIR;
      LOG_DEBUG("AIR");
    }
  }
  else if (attackCounter < ATTACK_CHARGE)
  {
    frame = ducking ? FRAME_ATTACK_DUCK : FRAME_ATTACK;
    LOG_DEBUG("ATTACK");
  }
  else
  {
    frame = ducking ? FRAME_ATTACK_CHARGE_DUCK : FRAME_ATTACK_CHARGE;
    LOG_DEBUG("ATTACK CHARGE");
  }

  sprites.drawPlusMask(playerX - SPRITE_ORIGIN_X - cameraX, playerY - SPRITE_ORIGIN_Y, player_plus_mask, frame + (flipped ? FRAME_FLIPPED_OFFSET : 0));

  if (attackCounter != 0 && attackCounter < ATTACK_CHARGE)
  {
    ab.drawFastHLine(playerX + (flipped ? -24 : 8) - cameraX , playerY - (ducking ? 3 : 11), 16);
  }
}

