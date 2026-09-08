#ifndef TREX_PTERODACTYL_HPP
#define TREX_PTERODACTYL_HPP
#include "assets.hpp"
namespace TRexGame {

// local include
// local include

#define PTERODACTYL_POSITION_Y1 15
#define PTERODACTYL_POSITION_Y2 25
#define PTERODACTYL_POSITION_Y3 35

static const BitmapMasked* const pterodactyl_sprites[] = { &pterodactyl_1, &pterodactyl_2 };
static const int8_t pterodactyl_y_positions[] = {PTERODACTYL_POSITION_Y1, PTERODACTYL_POSITION_Y2, PTERODACTYL_POSITION_Y2, PTERODACTYL_POSITION_Y3};

struct Pterodactyl : SpireScrollingToLeft {
  Pterodactyl(SpawnHold &spawnHolder) : SpireScrollingToLeft(pterodactyl_sprites[0], PTERODACTY_SPEED, PTERODACTYL_POSITION_Y1), spawnHolder(spawnHolder) { }

  virtual void step() override {
    SpireScrollingToLeft::step();
    animationStep();
    if(!isActive()) {
      if(respawnWait) {
        --respawnWait;
        return;
      }
      
      if(!spawnHolder.tryAcquire(this, PLAYER_SAFE_ZONE_WIDTH * 2))
        return;
      
      const uint16_t r = rand();
      position.y = pterodactyl_y_positions[r&3];
      respawnWait = scaleValue(r, PTERODACTY_RESPAWN_RATE/2) + PTERODACTY_RESPAWN_RATE/2;
      rearm();
    }
  }

private:
  uint8_t respawnWait = 0;
  SpawnHold &spawnHolder;

  uint8_t animationSkip = 0;
  void animationStep() {
    if(animationSkip) {
      --animationSkip;
      return;
    }
    animationSkip = 6;
    bitmap = bitmap == pterodactyl_sprites[0]? pterodactyl_sprites[1] : pterodactyl_sprites[0];
  }
};

}
#endif
