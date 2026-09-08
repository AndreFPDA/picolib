#ifndef TREX_HEART_LIVE_HPP
#define TREX_HEART_LIVE_HPP
#include "assets.hpp"
namespace TRexGame {

// local include
// local include

#define HEART_MIN_Y 15
#define HEART_DY 35

struct HeartLive : SpireScrollingToLeft {
  HeartLive() : SpireScrollingToLeft(&hearts_5x_bm, GROUND_CACTI_SCROLL_SPEED, HEART_MIN_Y) { limitRenderWidthTo = 7; }

  virtual void step() override {
    SpireScrollingToLeft::step();
    if(!isActive()) {
      if(respawnWait) {
        --respawnWait;
        return;
      }
      
      const uint16_t r = rand();
      respawnWait = SPAWN_NEW_LIVE_MIN_CYCLES + uint16_t(r&0xFF);
      position.y = HEART_MIN_Y + scaleValue(r>>6, HEART_DY);
      rearm();
    }
  }

  void eat() { position.x = -(int8_t)bitmap->width; }

private:
  uint16_t respawnWait = SPAWN_NEW_LIVE_MIN_CYCLES;
};

}
#endif
