#include "player.h"

#include "map.h"
#include "entity.h"
#include "assets.h"

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
Vec Player::pos;
bool Player::alive;

namespace
{
const Box normalHitbox =
{
  3, 14, // x, y
  6, 14  // width, height
};

const Box duckHitbox =
{
  3, 6, // x, y
  6, 6  // width, height
};

const Box knifeHitbox =
{
  0, 1, // x, y
  8, 4 // width, height
};

int8_t velocityX;
int16_t velocityYf;
// FIXME move flags to a single byte and use masking (is it worth it?)
bool grounded;
uint8_t attackCounter;
bool knifeAttack;
bool jumping;
bool ducking;
uint8_t knockbackCounter;
uint8_t levitateCounter;
bool flipped;
bool walkFrame;

bool knife;
bool knifeFlipped;
Vec knifePosition;
uint8_t knifeCounter;

} // unamed

void Player::init(int16_t x, int8_t y)
{
  pos.x = x;
  pos.y = y;
  grounded = false;
  attackCounter = 0;
  knifeAttack = false;
  alive = true;
  jumping = false;
  ducking = false;
  knockbackCounter = 0;
  levitateCounter = 0;
  velocityX = 0;
  velocityYf = 0;
  knife = false;
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
        alive = false;
      }
    }
  }

  // not alive
  if (!alive)
  {
    return;
  }

  // attack
  if (knockbackCounter == 0 && attackCounter == 0)
  {
    if (ab.justPressed(B_BUTTON))
    {
      attackCounter = ATTACK_TOTAL_DURATION;
      sound.tone(NOTE_GS4H, 10);
    }
    else if (ab.justPressed(UP_BUTTON))
    {
      knifeAttack = true;
      knifeCounter = KNIFE_DIST_MAX;
      attackCounter = ATTACK_TOTAL_DURATION;
      sound.tone(NOTE_GS5H, 10); // TODO another sfx
    }
  }

  // jump
  if (knockbackCounter == 0 && !ducking && attackCounter == 0 && grounded && ab.justPressed(A_BUTTON))
  {
    // start jumping
    grounded = false;
    jumping = true;
    velocityYf = -PLAYER_JUMP_FORCE_F;
  }

  // vertical movement: levitation (middle of jump)
  if (levitateCounter > 0)
  {
    --levitateCounter;
  }
  // vertical movement: jump
  else if (jumping)
  {
    velocityYf += PLAYER_JUMP_GRAVITY_F;
    if (velocityYf >= 0)
    {
      velocityYf = 0;
      jumping = false;
      levitateCounter = PLAYER_LEVITATE_DURATION;
    }
    else
    {
      Map::moveY(pos, velocityYf / F_PRECISION, ducking ? duckHitbox : normalHitbox);
    }
  }
  // vertical movement: walk
  else
  {
    velocityYf += PLAYER_FALL_GRAVITY_F;
    int16_t offsetY = velocityYf / F_PRECISION;
    if (offsetY > 0)
    {
      grounded = Map::moveY(pos, offsetY, ducking ? duckHitbox : normalHitbox);
    }
    else
    {
      grounded = Map::collide(pos.x, pos.y + 1, ducking ? duckHitbox : normalHitbox);
    }

    if (grounded)
    {
      velocityYf = 0;
    }
  }

  // horizontal movement: input
  if (knockbackCounter == 0)
  {
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
  }

  // horizontal movement: physic
  if (velocityX != 0 &&
      (
        // normal speed
        knockbackCounter == 0 && ab.everyXFrames(ducking ? PLAYER_SPEED_DUCK : PLAYER_SPEED_NORMAL) ||
        // knockback speed
        knockbackCounter > 0 && ab.everyXFrames(knockbackCounter < PLAYER_KNOCKBACK_FAST ? PLAYER_SPEED_KNOCKBACK_NORMAL : PLAYER_SPEED_KNOCKBACK_FAST)
      )
     )
  {
    if (!Map::collide(pos.x + velocityX, pos.y, ducking ? duckHitbox : normalHitbox))
    {
      pos.x += velocityX;
    }
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
      ducking = Map::collide(pos.x, pos.y, normalHitbox);
    }
  }

  // perform attack
  if (attackCounter > 0)
  {
    attackCounter--;
    if (attackCounter < ATTACK_CHARGE)
    {
      if (knifeAttack)
      {
        knife = true;
        knifePosition.x = pos.x + (flipped ? -14 : 6);
        knifePosition.y = pos.y - (ducking ? 4 : 12);
        knifeFlipped = flipped;
        attackCounter = 0;
        knifeAttack = false;
      }
      else
      {
        Entities::attack(pos.x + (flipped ? -24 : 0), pos.y - (ducking ? 3 : 11), pos.x + (flipped ? 0 : 24));
      }
    }
  }

  // check if player falled in a hole
  if (pos.y - SPRITE_ORIGIN_Y > 64)
  {
    alive = false;
  }

  // knife
  if(knife)
  {
    knifePosition.x += knifeFlipped ? -2 : 2;
    Entity* entity = Entities::collide(knifePosition, knifeHitbox);
    if(entity != NULL)
    {
      // TODO damage entity
      knife = false;
    }
    
    if(Map::collide(knifePosition.x, knifePosition.y, knifeHitbox))
    {
      knife = false;
    }
    
    if(--knifeCounter == 0)
    {
      knife = false;
    }
  }
  
  // check entity collision
  if (knockbackCounter == 0)
  {
    Entity* entity = Entities::collide(pos, ducking ? duckHitbox : normalHitbox);
    if (entity != NULL)
    {
      flipped = entity->pos.x < pos.x;
      velocityX = flipped ? 1 : -1;
      knockbackCounter = PLAYER_KNOCKBACK_DURATION;
      jumping = false;
      levitateCounter = 0;
      attackCounter = 0;
      if (--hp == 0)
      {
        alive = false;
      }
      flashCounter = 2;
      sound.tone(NOTE_GS3H, 25, NOTE_G3H, 15);
    }
  }
}

void Player::draw()
{
  uint8_t frame = 0;

  if (!alive)
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
      // when doing knife attack don't use the carge anim (it shows the leash)
      frame = knifeAttack ? FRAME_ATTACK : FRAME_ATTACK_CHARGE;
    }

    if (ducking)
    {
      frame++;
    }
  }

  sprites.drawPlusMask(pos.x - SPRITE_ORIGIN_X - Game::cameraX, pos.y - SPRITE_ORIGIN_Y, player_plus_mask, frame + (flipped ? FRAME_FLIPPED_OFFSET : 0));

  if (attackCounter != 0 && attackCounter < ATTACK_CHARGE)
  {
    sprites.drawPlusMask(pos.x + (flipped ? -24 : 8) - Game::cameraX , pos.y - (ducking ? 4 : 12), flipped ? player_attack_left_plus_mask : player_attack_right_plus_mask, 0);
  }

  if(knife)
  {
    sprites.drawPlusMask(knifePosition.x - Game::cameraX, knifePosition.y, flipped ? player_knife_left_plus_mask : player_knife_right_plus_mask, 0);
  }

#ifdef DEBUG_HITBOX
  Rect hitbox = ducking ? duckHitbox : normalHitbox;
  ab.fillRect(pos.x - hitbox.x - Game::cameraX, pos.y - hitbox.y, hitbox.width, hitbox.height);
#endif

}

