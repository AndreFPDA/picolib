#ifndef TREX_CORE_H
#define TREX_CORE_H
#include <stdint.h>

#include <../lib/oledfx/oledfx.hpp>
#include "../lib/oledfx/OLED_font.hpp"

namespace TRexGame {


/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef ARRAY_H_
#define ARRAY_H_

template <typename T, uint8_t SIZE>
struct array {
  T data[SIZE];
  constexpr uint8_t size() const {return SIZE;}
  constexpr operator T*() {return data;}
  constexpr operator const T*() const {return data;}
  template <typename IndexType>
  constexpr const T& operator [](const IndexType idx) const { return data[idx]; }
};

#endif /* ARRAY_H_ */

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _ENGINE_TYPES_H_
#define _ENGINE_TYPES_H_
// std include

/* General */

struct flash_uint8_t {
private:
  flash_uint8_t();
  uint8_t data;
};
static inline uint8_t flashByte(const flash_uint8_t* ptr, const uint16_t offset) {
  return pgm_read_byte(ptr + offset);
}

/* Geometry */

template<typename T>
struct Point2D {
    T x;
    T y;
};

//using Point2Du8 = Point2D<uint8_t>;
using Point2Di8 = Point2D<int8_t>;

/* Bitmaps and sprites */

struct BitmapMasked {
  uint8_t width;
  uint8_t height;
  const flash_uint8_t* data;
  const flash_uint8_t* mask;
  BitmapMasked(const uint8_t* const flashBitmap, const uint8_t* const flashMask, const uint8_t width, const uint8_t height) :
    width(width), height(height), data((const flash_uint8_t*)flashBitmap), mask((const flash_uint8_t*)flashMask) {}
  BitmapMasked(const uint8_t* const flashBitmap, const uint8_t* const flashMask) : 
    BitmapMasked((const flash_uint8_t*)flashBitmap, (const flash_uint8_t*)flashMask) {}
  BitmapMasked(const flash_uint8_t* const flashBitmap, const flash_uint8_t* const flashMask) :
    width(flashByte(flashBitmap, 0)), height(flashByte(flashBitmap, 1)), data(flashBitmap+2), mask(flashMask? flashMask+2 : 0) {}
};

struct Sprite {
  const BitmapMasked* bitmap = 0;
  Point2Di8 position{};
  enum AnchorCorner : uint8_t {
    ANCHOR_TOP_LEFT,
    ANCHOR_BOTTOM_LEFT,
  };
  AnchorCorner anchor;
  uint8_t limitRenderWidthTo = 0xFF;
  Sprite() {}
  Sprite(const BitmapMasked* const bitmap, const Point2Di8 &position, const AnchorCorner anchor = ANCHOR_TOP_LEFT) :
    bitmap(bitmap), position(position), anchor(anchor) {}
};

struct SpriteAnimated : Sprite {
  virtual void step() = 0;
protected:
  SpriteAnimated() {}
  SpriteAnimated(const BitmapMasked* const bitmap, const Point2Di8 &position, const AnchorCorner anchor = ANCHOR_TOP_LEFT) : Sprite(bitmap, position, anchor) {}
};

struct SpireScrollingToLeft : SpriteAnimated {
  SpireScrollingToLeft(const BitmapMasked* const bitmap, const uint8_t speed, const int8_t posinionY, const AnchorCorner anchor = ANCHOR_TOP_LEFT, const int8_t resetPosinionX = 127) : 
    SpriteAnimated(bitmap, {-127, posinionY}, anchor), speed(speed), resetPosinionX(resetPosinionX) {}

  virtual void step() override {
    for(uint8_t i = 0; isActive() && i < speed; ++i)
      --position.x;
  }
  bool isActive() const { return position.x > -(int8_t)bitmap->width;}
  void rearm() { position.x = resetPosinionX; }
protected:
  const uint8_t speed;
  const int8_t resetPosinionX;
};

struct Symbol {
  Symbol(const uint8_t* const flashBitmap, const uint8_t width, const uint8_t height, const uint8_t nSymbols) :
    cur(flashBitmap, 0, width, height), sprite(&cur, {0, 0}), bitmapBase((const flash_uint8_t*)flashBitmap), nSymbols(nSymbols) {}
  const Sprite& getSprite(uint8_t n, const Point2Di8 &position) {
    if(n >= nSymbols) n = 0;
    cur.data = bitmapBase + n*cur.width*((cur.height+7)/8);
    sprite.position = position;
    return sprite;
  }
  uint8_t getWidth() const { return cur.width; }
private:
  BitmapMasked cur;
  Sprite sprite;
  const flash_uint8_t* bitmapBase;
  uint8_t nSymbols;
};

/* Canvas */
struct BitCanvas {
  uint8_t* const bitmap;
  const uint8_t height;
  const uint8_t width;
  uint8_t xOffset = 0; //minus
  uint8_t yOffset = 0; //minus

  BitCanvas(uint8_t* const bitmapBuffer, const uint8_t height, const uint8_t width) :
    bitmap(bitmapBuffer), height(height), width(width) { clear(); }

  void clear(const bool v = false) {
    memset(bitmap, v? 0xFF : 0, width * height/8);
  }

  void render(const Sprite &sprite) {
    if(!sprite.bitmap) return;
    
    //sprite rectangle
    const int16_t sX0 = (int16_t)sprite.position.x - xOffset, sX1 = sX0 + min(sprite.bitmap->width, sprite.limitRenderWidthTo);
    const int16_t sY0 = (int16_t)sprite.position.y - (sprite.anchor == Sprite::ANCHOR_BOTTOM_LEFT? sprite.bitmap->height : 0) - yOffset, sY1 = sY0 + sprite.bitmap->height;
    if(sX0 >= width || sX1 <= 0 || sY0 >= height || sY1 <= 0) return;

    //cycle on canvas
    const uint8_t Xto = min((int)sX1, (int)width);
    const uint8_t Yto = min((int)sY1, (int)height);

    const uint8_t bitOffset = (sY0%8) + (sY0%8 < 0? 8 : 0);
    const uint8_t bitmapByteHeight = (sprite.bitmap->height + 7) / 8;
    
    for(uint8_t y = max((int)sY0, 0); y < 8*((Yto + 7)/8); y += 8) {
      const uint8_t yByte = y/8;
      const uint8_t sy = y - sY0;
      const uint8_t syByte = (sy + 7)/8; //allowed to be equal to bitmapByteHeight

      uint8_t* const bitmap_yw = bitmap + yByte*width;
      
      const flash_uint8_t* const sMask_yw =   //curren row mask, maybe null
        sprite.bitmap->mask && syByte < bitmapByteHeight  //mask exists and syByte is not out of array bounds
          ? sprite.bitmap->mask + syByte*sprite.bitmap->width 
          : 0;
      const flash_uint8_t* const sMask_yw_1 =   //previous row mask, maybe null
        sprite.bitmap->mask && syByte && (syByte-1) < bitmapByteHeight  //mask exists and syByte>0 and syByte-1 is not out of array bounds
          ? sprite.bitmap->mask + (syByte-1)*sprite.bitmap->width 
          : 0;
      const flash_uint8_t* const sData_yw =  //curren row data, maybe null
        syByte < bitmapByteHeight //syByte is not out of array bounds
          ? sprite.bitmap->data + syByte*sprite.bitmap->width
          : 0;
      const flash_uint8_t* const sData_yw_1 =  //previous row data, maybe null
        syByte && (syByte-1) < bitmapByteHeight //syByte>0 and syByte-1 is not out of array bounds
          ? sprite.bitmap->data + (syByte-1)*sprite.bitmap->width
          : 0;

      for(uint8_t x = max((int)sX0, 0); x < Xto; ++x) {
        //render 8 pixels
        const uint8_t sx = x - sX0;
        uint8_t& canvasByte = bitmap_yw[x];
        
        //apply mask
        if(sMask_yw_1) canvasByte &= ~(flashByte(sMask_yw_1, sx) >> (8-bitOffset));
        if(sMask_yw) canvasByte &= ~(flashByte(sMask_yw, sx) << bitOffset);
        //apply sprite bitmap data
        if(sData_yw_1) canvasByte |= flashByte(sData_yw_1, sx) >> (8-bitOffset);
        if(sData_yw) canvasByte |= flashByte(sData_yw, sx) << bitOffset;
      }
    }
  }
};

struct VirtualBitCanvas : BitCanvas {
  enum Mode : uint8_t {
    VIRTUAL_HEIGHT,
    VIRTUAL_WIDTH
  };
  VirtualBitCanvas(const Mode mode, uint8_t* const bitmapBuffer, const uint8_t bufferHeight, const uint8_t bufferWidth, const uint8_t virtualSize) :
    BitCanvas(bitmapBuffer, bufferHeight, bufferWidth), 
    virtualSize(virtualSize), 
    bufferSize(mode == VIRTUAL_HEIGHT? bufferHeight : bufferWidth), 
    offsetCoord(mode == VIRTUAL_HEIGHT? yOffset : xOffset) {}

  //returns true on wrap
  bool nextPart() {
    clear();
    if(offsetCoord + bufferSize >= virtualSize) {
      offsetCoord = 0;
      return true;
    }
    offsetCoord += bufferSize;
    return false;
  }
  
private:
  const uint8_t virtualSize;
  const uint8_t bufferSize;
  uint8_t &offsetCoord;
};

struct CollisionDetector {
  template<class DerivedFromSprite>
  static bool check(const Sprite &s, const DerivedFromSprite* const* ss, uint8_t size) {
    while(size--)
      if(check(s, **ss++)) return true;
    return false;
  }
    
  static bool check(const Sprite &s1, const Sprite &s2) {
    if(!s1.bitmap || !s2.bitmap) return false;
    
    const uint8_t width = s1.bitmap->width;
    const uint8_t height = s1.bitmap->height;
    //sprite 2 rectangle in sprite 1 frame
    const int16_t sX0 = (int16_t)s2.position.x - s1.position.x, sX1 = sX0 + min(s2.bitmap->width, s2.limitRenderWidthTo);
    const int16_t sY0 = (int16_t)s2.position.y - (s2.anchor == Sprite::ANCHOR_BOTTOM_LEFT? s2.bitmap->height : 0) - s1.position.y + (s1.anchor == Sprite::ANCHOR_BOTTOM_LEFT? s1.bitmap->height : 0), sY1 = sY0 + s2.bitmap->height;  
    if(sX0 >= width || sX1 <= 0 || sY0 >= height || sY1 <= 0) return false; //no rectangle intersection
    
    //cycle on sprite 1
    const uint8_t Xto = min((int)sX1, (int)width);
    const uint8_t Yto = min((int)sY1, (int)height);
    
    for(uint8_t y = max((int)sY0, 0); y < Yto; ++y) {
      const uint8_t yByte = y/8;
      const uint8_t yBit = y%8;
      const uint8_t sy = y - sY0;
      const uint8_t syByte = sy/8;
      const uint8_t syBit = sy%8;

      const flash_uint8_t* const s1data = s1.bitmap->data + yByte*width;
      const flash_uint8_t* const s2data = s2.bitmap->data + syByte*s2.bitmap->width;
      
      for(uint8_t x = max((int)sX0, 0); x < Xto; ++x) {
        const uint8_t sx = x - sX0;
        if((flashByte(s1data, x) & (1<<yBit)) && (flashByte(s2data, sx) & (1<<syBit))) return true;
      }
    }
    
    return false;
  }
private:
  CollisionDetector();
};

struct SpawnHold {
  bool tryAcquire(const void* me, const uint8_t countBeforeSpawn) {
    //Counter is free, acquire it and wait
    if(owner == 0) {
      counter = countBeforeSpawn;
      owner = me;
      return false;
    }
    //we're owning this counter, decrement or free it
    if(owner == me) {
      if(counter) {
        --counter;
        return false;
      } else {
        owner = 0;
        return true;
      }
    }
    return false; //someone else is owning the counter
  }
private:
  const void* owner = 0;
  uint8_t counter = 0;
};

static uint8_t scaleValue(const uint8_t val, const uint8_t limit) {
  return (uint16_t(val)*limit) >> 8;
}

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _ASSETS_H_
#define _ASSETS_H_

// local include

/* T-rex */
// local include
// local include
// local include
// local include
// local include
// local include
// local include
static const BitmapMasked trex_up_1(trex_up_1s_bitmap, trex_up_1s_mask);
static const BitmapMasked trex_up_2(trex_up_2s_bitmap, trex_up_2s_mask);
static const BitmapMasked trex_up_3(trex_up_3s_bitmap, trex_up_3s_mask);
static const BitmapMasked trex_duck_1(trex_duck_1s_bitmap, trex_duck_1s_mask);
static const BitmapMasked trex_duck_2(trex_duck_2s_bitmap, trex_duck_2s_mask);
static const BitmapMasked trex_dead_1(trex_dead_1s_no_outline_bitmap, trex_dead_1s_no_outline_mask);
static const BitmapMasked trex_dead_2(trex_dead_2s_no_outline_bitmap, trex_dead_2s_no_outline_mask);

/* Ground */
// local include
// local include
// local include
// local include
// local include
static const BitmapMasked ground_1(ground_1_bitmap, 0);
static const BitmapMasked ground_2(ground_2_bitmap, 0);
static const BitmapMasked ground_3(ground_3_bitmap, 0);
static const BitmapMasked ground_4(ground_4_bitmap, 0);
static const BitmapMasked ground_5(ground_5_bitmap, 0);

/* Cacti */
// local include
// local include
// local include
// local include
static const BitmapMasked cacti_2b(cacti_big_big_bitmap, cacti_big_big_mask);
static const BitmapMasked cacti_bs(cacti_big_small_bitmap, cacti_big_small_mask);
static const BitmapMasked cacti_sb(cacti_small_big_bitmap, cacti_small_big_mask);
static const BitmapMasked cacti_3s(cacti_small_small_small_bitmap, cacti_small_small_small_mask);

/* Pterodactyl */
// local include
// local include
static const BitmapMasked pterodactyl_1(pterodactyl_1_bitmap, pterodactyl_1_mask);
static const BitmapMasked pterodactyl_2(pterodactyl_2_bitmap, pterodactyl_2_mask);

/* Font */
const uint8_t font5x8_digits_data[] PROGMEM = {
  0x3E, 0x51, 0x49, 0x45, 0x3E,// 0 0x30
  0x00, 0x42, 0x7F, 0x40, 0x00,// 1 0x31
  0x42, 0x61, 0x51, 0x49, 0x46,// 2 0x32
  0x21, 0x41, 0x45, 0x4B, 0x31,// 3 0x33
  0x18, 0x14, 0x12, 0x7F, 0x10,// 4 0x34
  0x27, 0x45, 0x45, 0x45, 0x39,// 5 0x35
  0x3C, 0x4A, 0x49, 0x49, 0x30,// 6 0x36
  0x01, 0x71, 0x09, 0x05, 0x03,// 7 0x37
  0x36, 0x49, 0x49, 0x49, 0x36,// 8 0x38
  0x06, 0x49, 0x49, 0x29, 0x1E // 9 0x39  
};
static Symbol numbers(font5x8_digits_data, 5, 8, 10);

/* Text and icons */
// local include
// local include
static const BitmapMasked game_overver_bm(game_over_bitmap, game_over_mask);
// local include
static const BitmapMasked restart_icon_bm(restart_icon_bitmap, restart_icon_mask);
// local include
static const BitmapMasked hearts_5x_bm(hearts_5x_bitmap, 0);

const uint8_t hi_score_bitmap[] PROGMEM = {
  11, //width
  8,  //height
  0x7F, 0x08, 0x08, 0x08, 0x7F,// H
  0x00,
  0x00, 0x41, 0x7F, 0x41, 0x00,// I
};
static const BitmapMasked hi_score(hi_score_bitmap, 0);

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _TREX_PLAYER_H_
#define _TREX_PLAYER_H_

// local include
// local include

#define T_REX_JUMP_MOMENTUM 8 //jump height = 6 -> 21px, 7 -> 28px, 8 -> 36px
#define T_REX_JUMP_MOMENTUM_DUCKED 6
#define T_REX_START_POINT {5, 60}

static const BitmapMasked* const trex_sprites[] = {&trex_up_1, &trex_up_2, &trex_up_3, &trex_duck_1, &trex_duck_2, &trex_dead_1, &trex_dead_2};
#define T_REX_SPRITE_UP_START 0
#define T_REX_SPRITE_UP_END 3
#define T_REX_SPRITE_DUCK_START 3
#define T_REX_SPRITE_DUCK_END 5
#define T_REX_SPRITE_DEAD_UP 5
#define T_REX_SPRITE_DEAD_DUCK 6

struct TrexPlayer : SpriteAnimated {
  enum State : uint8_t {
    UP,
    DUCK,
    DEAD
  };
  State state = UP;
  TrexPlayer() : SpriteAnimated(trex_sprites[0], T_REX_START_POINT, ANCHOR_BOTTOM_LEFT) {}

  virtual void step() override {
    animationStep();
    motionStep();
  }

  void jump() {
      if(isJumping() || state == DEAD) return;
      vy = state == UP? T_REX_JUMP_MOMENTUM : T_REX_JUMP_MOMENTUM_DUCKED;
  }
  void duck(const bool toDuck) {
    if(toDuck) {
      if(state == UP && !isJumping()) state = DUCK;
    } else {
      if(state == DUCK) state = UP;
    }
  }
  void die() {
    bitmap = trex_sprites[state == DUCK? T_REX_SPRITE_DEAD_DUCK : T_REX_SPRITE_DEAD_UP];
    state = DEAD;
    vy = 0;
  }
  void blink() { blinkCnt = PLAYER_SAFE_ZONE_WIDTH; }
  bool isBlinking() { return blinkCnt; }

protected:
  //motion
  int8_t dy = 0;
  int8_t vy = 0;
  bool skipStep = false;
  bool isJumping() const { return dy != 0 || vy != 0; }
  void motionStep() {
    //dirty fix to prolong the jump
    if(abs(vy) <= 1 && !skipStep) {
      skipStep = true;
      return;
    }
    skipStep = false;
    
    dy += vy;
    position.y -= vy;
    if(dy) --vy;
    else vy = 0;
  }

  //animation
  uint8_t bitmapId = 0;
  uint8_t blinkCnt = 0;
  void animationStep() {
    if(blinkCnt) --blinkCnt;
    if(blinkCnt & 1) {
      bitmap = 0;
      return;
    }
    
    uint8_t start = 0, end = 0;

    if(state == UP)
      start = T_REX_SPRITE_UP_START, end = isJumping()? T_REX_SPRITE_UP_START : T_REX_SPRITE_UP_END;
    else if(state == DUCK)
      start = T_REX_SPRITE_DUCK_START, end = isJumping()? T_REX_SPRITE_DUCK_START : T_REX_SPRITE_DUCK_END;
    else if(state == DEAD)
      start = T_REX_SPRITE_DEAD_UP, end = T_REX_SPRITE_DEAD_UP;

    if(!(bitmapId >= start && bitmapId < end)) bitmapId = start;
    if(bitmapId + 1 < end) ++bitmapId;
    else bitmapId = start;
    
    bitmap = trex_sprites[bitmapId];
  }
};

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _PTERODACTYL_H_
#define _PTERODACTYL_H_

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

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _GROUND_H_
#define _GROUND_H_

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

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _HEART_LIVE_H_
#define _HEART_LIVE_H_

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

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _CACTUS_H_
#define _CACTUS_H_

// local include
// local include

#define CACTUS_POSITION_Y 61

static const BitmapMasked* const cacti_sprites[] = {
  &cacti_sb, &cacti_sb,
  &cacti_bs, &cacti_bs,
  &cacti_2b, &cacti_2b,
  &cacti_3s, &cacti_3s,
};
static const uint8_t cacti_width[] = { 
  10, cacti_sb.width,
  14, cacti_bs.width,
  14, cacti_2b.width,
  18, cacti_3s.width
};

struct Cactus : SpireScrollingToLeft {
  Cactus(SpawnHold &spawnHolder) : SpireScrollingToLeft(cacti_sprites[0], GROUND_CACTI_SCROLL_SPEED, CACTUS_POSITION_Y, ANCHOR_BOTTOM_LEFT), spawnHolder(spawnHolder) { /*position.x = 59;*/ }

  virtual void step() override {
    SpireScrollingToLeft::step();
    if(!isActive()) {
      if(respawnWait) {
        --respawnWait;
        return;
      }
      
      if(!spawnHolder.tryAcquire(this, PLAYER_SAFE_ZONE_WIDTH))
        return;
            
      const uint16_t r = rand();
      const uint8_t i = r&7;
      bitmap = cacti_sprites[i];
      limitRenderWidthTo = cacti_width[i];
      respawnWait = scaleValue(r, CACTI_RESPAWN_RATE);
      rearm();
    }
  }

private:
  uint8_t respawnWait = 0;
  SpawnHold &spawnHolder;
};

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _I2C_CALSS_H_
#define _I2C_CALSS_H_

#if defined(ESP8266) || defined(ESP32)

// std include

struct I2C {
  I2C() {}
  
  static void init(const uint32_t clock) {
    Wire.begin();
    Wire.setClock(clock);
  }
  
  static void deinit(void) {
    // Standard Wire library doesn't need deinit here
  }

  static uint8_t start(uint8_t address) {
    Wire.beginTransmission(address >> 1);
    return 0;
  }

  static void stop(void) {
    Wire.endTransmission();
  }
  
  static uint8_t write(uint8_t data) {  
    Wire.write(data);
    return 0;
  }

  static uint8_t readAck(void) {
    return 0; // Not used
  }
  
  static uint8_t readNak(void) {
    return 0; // Not used
  }
};

#else
// --- Standard AVR Implementation ---

// std include

struct I2C {
  I2C() {}
  
  static void init(const uint32_t clock) {
    TWSR = 0;                         // no prescaler
    TWBR = ((F_CPU/clock)-16)/2;  // must be > 10 for stable operation
  }
  
  static void deinit(void) {
    TWBR = 0;
    TWCR = 0;
  }

  static uint8_t start(uint8_t address) {
    uint8_t   twst, i;
    // send START condition
    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
  
    // wait until transmission completed
    //while(!(TWCR & (1<<TWINT)));
    for(i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    if(!i) return 0xFF;
  
    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
    if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
  
    // send device address
    TWDR = address;
    TWCR = (1<<TWINT) | (1<<TWEN);
  
    // wail until transmission completed and ACK/NACK has been received
    //while(!(TWCR & (1<<TWINT)));
    for(i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    if(!i) return 0xFF;
    
    // check value of TWI Status Register. Mask prescaler bits.
    twst = TW_STATUS & 0xF8;
    if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
  
    return 0;
  }

  static void stop(void) {
    //send stop condition
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
    
    // wait until stop condition is executed and bus released
    //while(TWCR & (1<<TWSTO));
    for(uint8_t i = 255; i; --i) {
      if(!(TWCR & (1<<TWSTO))) break;
      asm volatile("nop");
      asm volatile("nop");
    }
  }
  
  static uint8_t write(uint8_t data) {  
    uint8_t   twst;
      
    // send data to the previously addressed device
    TWDR = data;
    TWCR = (1<<TWINT) | (1<<TWEN);
  
    // wait until transmission completed
    //while(!(TWCR & (1<<TWINT)));
    uint8_t i;
    for(i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    if(!i) return 0xFF;
    
    // check value of TWI Status Register. Mask prescaler bits
    twst = TW_STATUS & 0xF8;
    if( twst != TW_MT_DATA_ACK) return twst;
    return 0;
  }

  static uint8_t readAck(void) {
    TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
    //while(!(TWCR & (1<<TWINT)));
    for(uint8_t i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    return TWDR;
  }
  
  static uint8_t readNak(void) {
    TWCR = (1<<TWINT) | (1<<TWEN);
    //while(!(TWCR & (1<<TWINT)));
    for(uint8_t i = 255; i; --i) {
      if(TWCR & (1<<TWINT)) break;
      asm volatile("nop");
      asm volatile("nop");
    }
    return TWDR;
  }
};

#endif

#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _SH1106_H_
#define _SH1106_H_

#define SH1106_I2C_SCL_CLOCK  800000UL
#define SH1106_I2C_ADDR 0b01111000
#define SH1106_COMMAND 0x00
#define SH1106_DATA 0x40

#define SH1106_COLS_USED 128
#define SH1106_PAGES 8

#ifndef LCD_SSD1306
  #define SH1106_COLS_TOTAL 130
#else 
  #define SH1106_COLS_TOTAL SH1106_COLS_USED
#endif

template<class I2C_TYPE>
class SH1106 {
  I2C_TYPE &i2c;
  const uint16_t screenBufferSize;
public:
  enum AddressingMode {
    HorizontalAddressingMode = 0x00,
    VerticalAddressingMode = 0x01,
    PageAddressingMode = 0x02,
  };
  SH1106(I2C_TYPE &i2c, const uint16_t screenBufferSize): 
    i2c(i2c), screenBufferSize(screenBufferSize) {}
  void begin() {
    col = 0;
    page = 0;
    inverted = false;
#if defined(LCD_SSD1306)
    i2c.deinit();
    i2c.init(SH1106_I2C_SCL_CLOCK);
    sendc(0xAE);             // display off
    sendc(0xD5, 0x80);       // clock divide
    sendc(0xA8, 0x3F);       // multiplex ratio (64)
    sendc(0xD3, 0x00);       // display offset
    sendc(0x40);             // start line
    sendc(0x8D, 0x14);       // charge pump enable
    sendc(0x20, 0x00);       // horizontal addressing
    sendc(0xA1);             // segment remap
    sendc(0xC8);             // COM scan direction
    sendc(0xDA, 0x12);       // COM pins
    sendc(0x81, 0xCF);       // contrast
    sendc(0xD9, 0xF1);       // pre-charge
    sendc(0xDB, 0x40);       // VCOMH deselect
    sendc(0xA4);             // display on resume
    sendc(0xA6);             // normal (non-inverted)
    sendc(0xAF);             // display on
#endif
  }
  void fillScreen(const uint8_t* buffer) {
    reinit();
    sendd(buffer, screenBufferSize);
  }
  void fillScreen(const uint8_t* buffer, const uint16_t size, const uint8_t stride = 0) { //stride is not supported
    reinit();
    sendd(buffer, size);
  }
  void setInverse(const bool v) {
    inverted = v;
    reinit();
  }
  //Only PageAddressingMode works for SH1106
  //Here we're emulating HorizontalAddressingMode programmatically
  void setAddressingMode(const AddressingMode addressingMode) {} 
  
private:
  /* lcd control */
  void reinit() {
    i2c.deinit();
    i2c.init(SH1106_I2C_SCL_CLOCK);
    sendc(0xAF); //display on
    sendc(inverted? 0xA7 : 0xA6); //inversion
  }
  
  /* send bytes */
  void sendc(const uint8_t cmd) {
    i2c.start(SH1106_I2C_ADDR);
    i2c.write(SH1106_COMMAND);
    i2c.write(cmd);
    i2c.stop();
  }
  void sendc(const uint8_t cmd1, const uint8_t cmd2) {
    i2c.start(SH1106_I2C_ADDR);
    i2c.write(SH1106_COMMAND);
    i2c.write(cmd1);
    i2c.write(cmd2);
    i2c.stop();
  }
  void sendc(const uint8_t cmd1, const uint8_t cmd2, const uint8_t cmd3) {
    i2c.start(SH1106_I2C_ADDR);
    i2c.write(SH1106_COMMAND);
    i2c.write(cmd1);
    i2c.write(cmd2);
    i2c.write(cmd3);
    i2c.stop();
  }
  void sendd(const uint8_t* d, uint16_t sz) {  
    const uint8_t blackPx = inverted? 0xFF : 0;
    while(sz) {  //page cycle
      sendc(0xB0 + page); //set page address to i (0..7)
      sendc(0x00 + (col&0x0F)); //Sets 4 lower bits of column address 
      sendc(0x10 + (col>>4)); //Sets 4 higher bits of column address 
      
      i2c.start(SH1106_I2C_ADDR);
      i2c.write(SH1106_DATA);
      while(sz) { //column cycle
        i2c.write(*d++);
        --sz;
        if(++col >= SH1106_COLS_USED) {
          while(col++ < SH1106_COLS_TOTAL) i2c.write(blackPx);
          col = 0;
          break;
        }
      }
      i2c.stop();
    
      if(col == 0 && ++page >= SH1106_PAGES) page = 0;
    }     
  }

  uint8_t col = 0;
  uint8_t page = 0;
  bool inverted = false;
};


#endif

/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome brower rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * E-mail: endoftheworld@bk.ru
 * License: MIT
*/ 

#ifndef _SSD1309_H_
#define _SSD1309_H_

template<class SPI_TYPE>
class SSD1309 {
  SPI_TYPE &spi;
  const uint8_t cs;
  const uint8_t dc;
  const uint8_t res;
  const uint16_t screenBufferSize;
public:
  enum AddressingMode {
    HorizontalAddressingMode = 0x00,
    VerticalAddressingMode = 0x01,
    PageAddressingMode = 0x02,
  };
  SSD1309(SPI_TYPE &spi, const uint8_t cs, const uint8_t dc, const uint8_t res, const uint16_t screenBufferSize): 
    spi(spi), cs(cs), dc(dc), res(res), screenBufferSize(screenBufferSize) {}
  void begin() {
    pinMode(cs, OUTPUT);
    digitalWrite(cs, HIGH);
    pinMode(dc, OUTPUT);
    pinMode(res, OUTPUT);
    digitalWrite(res, LOW);
    delay(50);
    digitalWrite(res, HIGH);
    spi.begin();
    init();
  }
  void fillScreen(const uint8_t* buffer) {
    sendd(buffer, screenBufferSize);
  }
  void fillScreen(const uint8_t* buffer, const uint16_t size, const uint8_t stride = 0) {
    sendd(buffer, size, stride);
  }
  void setInverse(const bool v) {
    sendc(v? 0xA7 : 0xA6);
  }
  void setAddressingMode(const AddressingMode addressingMode) {
    sendc(0x20, addressingMode);
  }
private:
  /* lcd control */
  void init() {
    sendc(0xAF);   //Set Display ON
    //sendc(0xA5, 1); //Entire Display ON
    //sendc(0xA7);  //Inverse Mode

    //sendc(0x20, addressingMode); //Set Addressing Mode
  }
  /* send bytes */
  void sendc(const uint8_t c1) {
    sendBytes(&c1, 1, false);
  }
  void sendc(const uint8_t c1, const uint8_t c2) { 
    const uint8_t c[] = {c1, c2}; 
    sendBytes(c, sizeof(c), false);
  }
  void sendc(const uint8_t c1, const uint8_t c2, const uint8_t c3) { 
    const uint8_t c[] = {c1, c2, c3}; 
    sendBytes(c, sizeof(c), false);
  }
  void sendd(const uint8_t* d, uint16_t sz, const uint8_t stride = 0) {
    sendBytes(d, sz, true, stride);
  }
  void sendBytes(const uint8_t* d, uint16_t sz, const bool data, const uint8_t stride = 0) {
    digitalWrite(dc, data);
    digitalWrite(cs, LOW);
    if(!stride)
      while(sz--) spi.transfer(*d++);
    else
      for(uint8_t i = 0; i < stride; ++i)
        for(uint16_t j = i; j < sz; j += stride)
          spi.transfer(d[j]);
    digitalWrite(cs, HIGH);
  }
};

#endif

} // namespace TRexGame
#endif
