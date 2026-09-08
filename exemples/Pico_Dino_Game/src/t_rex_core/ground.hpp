#ifndef TREX_GROUND_HPP
#define TREX_GROUND_HPP
#include "assets.hpp"
namespace TRexGame {

// local include
// local include

#define GROUND_POSITION_Y 64
static const BitmapMasked* const ground_sprites[] = {&ground_1, &ground_2, &ground_3, &ground_4, &ground_5, &ground_1, &ground_2, &ground_3};

struct Ground : SpireScrollingToLeft {
  Ground(const int8_t startX) : SpireScrollingToLeft(ground_sprites[0], GROUND_CACTI_SCROLL_SPEED, GROUND_POSITION_Y, ANCHOR_BOTTOM_LEFT) { position.x = startX; }

  virtual void step() override {
    for(uint8_t i = 0; i < speed; ++i) {
      --position.x;
      if(!isActive()) {
        bitmap = ground_sprites[rand()&7];
        rearm();
      }
    }
  }
};

}
#endif
