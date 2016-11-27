#include "player.h"

#include "entity.h"
#include "data.h"
#include "map.h"
#include "candle.h"
#include "assets.h"

// FIXME when ducking and under a tile, prevent player from standing
// --> actually this makes walking while ducking easier
// --> is there even a way to prevent player from having to press down + dir

// FIXME candle collision is still buggy

// TODO refactor attack so that it freeze player when performing (stay in air, don't stand if duck, ...)
// TODO refactor attack and animations

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

Rect normalHitbox;
Rect duckHitbox;

const uint8_t idleAnim[] = {LOOP, /* FRAME_RATE */ 0, /* FRAME_COUNT */ 1, /* FRAMES */ 0};
const uint8_t walkAnim[] = {LOOP, /* FRAME_RATE */ 12, /* FRAME_COUNT */ 2, /* FRAMES */ 0, 1};
const uint8_t attackAnim[] = {ONE_SHOT, /* FRAME_RATE */ 10, /* FRAME_COUNT */ 2, /* FRAMES */ 2, 3, 3};
const uint8_t airAnim[] = {LOOP, /* FRAME_RATE */ 0, /* FRAME_COUNT */ 1, /* FRAMES */ 4};
const uint8_t duckIdleAnim[] = {LOOP, /* FRAME_RATE */ 0, /* FRAME_COUNT */ 1, /* FRAMES */ 5};
const uint8_t duckAttackAnim[] = {ONE_SHOT, /* FRAME_RATE */ 10, /* FRAME_COUNT */ 2, /* FRAMES */ 6, 7, 7};

bool moveX(int16_t dx, const Rect& hitbox)
{
  if (dx != 0)
  {
    int8_t sign = dx > 0 ? 1 : -1;
    while (dx != 0)
    {
      if(Map::collide(p.x + sign, p.y, hitbox))
      {
        return true;
      }
      p.x += sign;
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
      if(Map::collide(p.x, p.y + sign, hitbox))
      {
        return true;
      }
      p.y += sign;
      dy -= sign;
    }
  }

  return false;
}

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
      grounded = Map::collide(p.x, p.y + 1, ducking ? duckHitbox : normalHitbox);
    }

    if (grounded)
    {
      velocityY = 0;
    }
  }

  //LOG_DEBUG(grounded ? "GROUNDED" : "AIR");

  // horizontal movement
  if (!attacking && ab.pressed(LEFT_BUTTON))
  {
    velocityX = -1;
    p.flipped = true;
  }
  else if (!attacking && ab.pressed(RIGHT_BUTTON))
  {
    velocityX = 1;
    p.flipped = false;
  }
  else if (grounded)
  {
    velocityX = 0;
  }

  if (ab.everyXFrames(ducking ? 4 : 2) && velocityX != 0)
  {
    moveX(velocityX, ducking ? duckHitbox : normalHitbox);
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
      ab.drawFastHLine(p.x + (p.flipped ? -24 : 8) - cameraX , p.y - (ducking ? 3 : 11), 16);
      // BAD BAD BAD
      Candles::hit(p.x + (p.flipped ? -20 : 4), p.y - (ducking ? 3 : 11), 20);
    }
  }
}

