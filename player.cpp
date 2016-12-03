#include "player.h"

#include "map.h"
#include "entity.h"
#include "assets.h"

// FIXME Cannot fall in a single tile

#define ATTACK_TOTAL_DURATION 16
#define ATTACK_CHARGE 10

#define SPRITE_ORIGIN_X 8
#define SPRITE_ORIGIN_Y 16

#define FRAME_IDLE 0
#define FRAME_WALK_1 0
#define FRAME_WALK_2 2
#define FRAME_ATTACK_CHARGE 4
#define FRAME_ATTACK 6
#define FRAME_AIR 8
#define FRAME_KNOCKBACK 9
#define FRAME_DEAD 10

#define FRAME_FLIPPED_OFFSET 11

#define WALK_FRAME_RATE 12

uint8_t Player::hp;
uint16_t Player::score;

namespace
{
const Rect normalHitbox =
{
  4, 14, // x, y
  8, 14  // width, height
};

const Rect duckHitbox =
{
  4, 6, // x, y
  8, 6  // width, height
};

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
uint8_t knockbackCounter;
int8_t levitateCounter;
bool flipped;
bool walkFrame;
bool dead;

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

//bool moveY(int16_t dy, const Rect& hitbox)
//{
//  if (dy != 0)
//  {
//    int8_t sign = dy > 0 ? 1 : -1;
//    while (dy != 0)
//    {
//      if (Map::collide(playerX, playerY + sign, hitbox))
//      {
//        return true;
//      }
//      playerY += sign;
//      dy -= sign;
//    }
//  }
//
//  return false;
//}

} // unamed

void Player::init(int16_t x, int16_t y)
{
  playerX = x;
  playerY = y;
  grounded = false;
  attackCounter = 0;
  attacking = false;
  jumping = false;
  ducking = false;
  dead = false;
  knockbackCounter = 0;
  levitateCounter = 0;
  velocityX = 0;
  velocityY = 0;
}

void Player::update()
{
  // knockback
  if (knockbackCounter > 0)
  {
    if (--knockbackCounter == 0)
    {
      velocityX = 0;
      if (hp == 0)
      {
        dead = true;
      }
    }
  }

  if (dead)
  {
    return;
  }

  // attack
  if (knockbackCounter == 0 && attackCounter == 0 && ab.justPressed(B_BUTTON))
  {
    attackCounter = ATTACK_TOTAL_DURATION;
    sound.tone(NOTE_GS4, 10);
  }

  if (attackCounter > 0)
  {
    --attackCounter;
  }

  // jump
  if (knockbackCounter == 0 && !ducking && attackCounter == 0 && grounded && ab.justPressed(A_BUTTON))
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
      levitateCounter = PLAYER_LEVITATE_DURATION;
    }
    else
    {
      Map::moveY(playerX, playerY, velocityY / F_PRECISION, ducking ? duckHitbox : normalHitbox);
    }
  }
  else
  {
    velocityY += PLAYER_FALL_GRAVITY_F;
    int16_t offsetY = velocityY / F_PRECISION;
    if (offsetY > 0)
    {
      grounded = Map::moveY(playerX, playerY, offsetY, ducking ? duckHitbox : normalHitbox);
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
  if (knockbackCounter == 0)
  {
    if (attackCounter == 0 && ab.pressed(LEFT_BUTTON))
    {
      velocityX = -1;
      //verticalMoveCounter = 0;
      flipped = true;
    }
    else if (attackCounter == 0 && ab.pressed(RIGHT_BUTTON))
    {
      velocityX = 1;
      // verticalMoveCounter = 0;
      flipped = false;
    }
    else if (grounded)
    {
      velocityX = 0;
    }
  }

  if (velocityX != 0 &&
      (
        knockbackCounter == 0 && ab.everyXFrames(ducking ? PLAYER_SPEED_DUCK : PLAYER_SPEED_NORMAL) ||
        knockbackCounter > 0 && ab.everyXFrames(knockbackCounter < PLAYER_KNOCKBACK_FAST ? PLAYER_SPEED_KNOCKBACK_NORMAL : PLAYER_SPEED_KNOCKBACK_FAST)
      )
     )
  {
    moveX(velocityX, ducking ? duckHitbox : normalHitbox);
  }

  // duck
  if (knockbackCounter == 0 && attackCounter == 0)
  {
    if (!ducking)
    {
      ducking = grounded && ab.pressed(DOWN_BUTTON);
    }
    else if (!ab.pressed(DOWN_BUTTON))
    {
      // only stop ducking if player can stand
      ducking = Map::collide(playerX, playerY, normalHitbox);
    }

    // verticalMoveSpeed = ducking ? PLAYER_SPEED_DUCK : PLAYER_SPEED_NORMAL;
  }

  // perform attack
  if (attackCounter != 0 && attackCounter < ATTACK_CHARGE)
  {
    Entities::attack(playerX + (flipped ? -24 : 0), playerY - (ducking ? 3 : 11), 24);
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
    if (cameraX > Map::width * TILE_WIDTH - 128) cameraX = Map::width * TILE_WIDTH - 128;
  }

  // check for out of map conditions
  if (playerX - normalHitbox.x > Map::width * TILE_WIDTH)
  {
    Game::init(); // TODO
  }
  else if (playerY - SPRITE_ORIGIN_Y > 64)
  {
    Game::init(); // TODO
  }

  // check entity collision
  if (knockbackCounter == 0)
  {
    Entity* entity = Entities::collide(playerX, playerY, ducking ? duckHitbox : normalHitbox);
    if (entity != NULL)
    {
      flipped = entity->x < playerX;
      velocityX = flipped ? 1 : -1;
      knockbackCounter = PLAYER_KNOCKBACK_DURATION;
      jumping = false;
      levitateCounter = 0;
      attackCounter = 0;
      --hp;
      sound.tone(NOTE_GS3, 25, NOTE_G3, 15);
    }
  }
}

void Player::draw()
{
  uint8_t frame = 0;

  if (dead)
  {
    frame = FRAME_DEAD;
  }
  else if (knockbackCounter > 0)
  {
    frame = FRAME_KNOCKBACK;
  }
  else if (attackCounter == 0 && !grounded)
  {
    frame = FRAME_AIR;
  }
  else
  {
    if (attackCounter == 0)
    {
      if (velocityX == 0)
      {
        frame = FRAME_IDLE;
      }
      else
      {
        if (ab.everyXFrames(WALK_FRAME_RATE))
        {
          walkFrame = !walkFrame;
        }
        frame = walkFrame ? FRAME_WALK_2 : FRAME_WALK_1;
      }
    }
    else if (attackCounter < ATTACK_CHARGE)
    {
      frame = FRAME_ATTACK;
    }
    else
    {
      frame = FRAME_ATTACK_CHARGE;
    }

    if (ducking)
    {
      frame++;
    }
  }

  sprites.drawPlusMask(playerX - SPRITE_ORIGIN_X - cameraX, playerY - SPRITE_ORIGIN_Y, player_plus_mask, frame + (flipped ? FRAME_FLIPPED_OFFSET : 0));

  if (attackCounter != 0 && attackCounter < ATTACK_CHARGE)
  {
    sprites.drawPlusMask(playerX + (flipped ? -24 : 8) - cameraX , playerY - (ducking ? 4 : 12), flipped ? player_attack_left_plus_mask : player_attack_right_plus_mask, 0);
  }

#ifdef DEBUG_HITBOX
  Rect hitbox = ducking ? duckHitbox : normalHitbox;
  ab.fillRect(playerX - hitbox.x - cameraX, playerY - hitbox.y, hitbox.width, hitbox.height);
#endif

}

