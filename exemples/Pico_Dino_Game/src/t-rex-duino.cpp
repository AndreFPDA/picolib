/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome browser rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * Standalone adaptation for ESP01 / Arduino
 */

#include "t-rex-duino.hpp"
#include "t_rex_core/TRexCore.h"
#include "pico/stdlib.h"
#include "pico/rand.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define TOUCH_SENSOR_PIN 5
#define BUZZER_PIN 12
#define LOW false
#define HIGH true

extern "C" void tud_task(void);

static bool digitalRead(uint pin) {
  return gpio_get(pin);
}

static unsigned long millis() {
  return to_ms_since_boot(get_absolute_time());
}

static void noTone(uint pin) {
  pwm_set_enabled(pwm_gpio_to_slice_num(pin), false);
}

static void tone(uint pin, uint frequency) {
  const uint slice = pwm_gpio_to_slice_num(pin);
  gpio_set_function(pin, GPIO_FUNC_PWM);
  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, 1.0f);
  pwm_config_set_wrap(&config, 125000000 / frequency - 1);
  pwm_init(slice, &config, true);
  pwm_set_gpio_level(pin, (125000000 / frequency) / 2);
}

static void serviceUsb() {
  tud_task();
}

namespace t_rex {

/* Sound effects */
const BuzzerNote SND_MENU_BACK[] = {
  { 2400, 30 }, { 0, 15 }, { 1800, 40 }
};

const BuzzerNote SND_DINO_JUMP[] = {
  { 1000, 8 }, { 0, 4 }, { 1400, 8 }, { 0, 4 }, { 1800, 12 }
};

const BuzzerNote SND_DINO_HIT[] = {
  { 900, 25 }, { 0, 10 }, { 700, 35 }
};

const BuzzerNote SND_DINO_GAMEOVER[] = {
  { 800, 60 }, { 0, 15 }, { 600, 60 }, { 0, 15 }, { 400, 60 }, { 0, 15 }, { 300, 100 }
};

const BuzzerNote SND_DINO_HEART[] = {
  { 1100, 10 }, { 0, 5 }, { 1400, 10 }, { 0, 5 }, { 1700, 14 }
};

#ifndef PLAY
#define PLAY(s) buzzer_play(s, sizeof(s) / sizeof(s[0]))
#endif

/* Buzzer engine */
static const BuzzerNote* _seq = nullptr;
static int _seqLen = 0;
static int _noteIdx = 0;
static unsigned long _noteStart = 0;
static bool _buzPlaying = false;

void buzzer_update() {
  if (!_buzPlaying) return;
  unsigned long now = millis();
  if (now - _noteStart < (unsigned long)_seq[_noteIdx].dur) return;
  _noteIdx++;
  if (_noteIdx >= _seqLen) {
    noTone(BUZZER_PIN);
    _buzPlaying = false;
    return;
  }
  _noteStart = now;
  (_seq[_noteIdx].freq == 0) ? noTone(BUZZER_PIN)
                             : tone(BUZZER_PIN, _seq[_noteIdx].freq);
}

void buzzer_play(const BuzzerNote* seq, int len) {
  _seq = seq;
  _seqLen = len;
  _noteIdx = 0;
  _noteStart = millis();
  _buzPlaying = true;
  (seq[0].freq == 0) ? noTone(BUZZER_PIN) : tone(BUZZER_PIN, seq[0].freq);
}

bool buttonPressed() { return digitalRead(TOUCH_SENSOR_PIN) == LOW; }

/* Touch/click state */
bool singleClick = false;
bool doubleClick = false;
bool tripleClick = false;
bool quadClick = false;
bool longPress = false;

static bool _stableState = LOW;
static bool _lastReading = LOW;
static bool _longPressFired = false;
static bool _multiClickWaiting = false;
static uint8_t _clickCount = 0;
static unsigned long _lastDebounceTime = 0;
static unsigned long _pressStartTime = 0;
static unsigned long _lastReleaseTime = 0;

#define DEBOUNCE_TIME 20
#define MULTI_CLICK_GAP 250
#define LONG_PRESS_TIME 500

void updateTouch() {
  singleClick = false;
  doubleClick = false;
  tripleClick = false;
  quadClick = false;
  longPress = false;

  bool reading = buttonPressed();

  if (reading != _lastReading) {
    _lastDebounceTime = millis();
    _lastReading = reading;
  }

  if ((millis() - _lastDebounceTime) > DEBOUNCE_TIME) {
    if (reading != _stableState) {
      _stableState = reading;
      if (_stableState == HIGH) {
        _pressStartTime = millis();
        _longPressFired = false;
        if ((millis() - _lastReleaseTime) > 2000) {
          _clickCount = 0;
          _multiClickWaiting = false;
        }
      } else {
        if (!_longPressFired) {
          _clickCount++;
          _multiClickWaiting = true;
          _lastReleaseTime = millis();
        }
      }
    }
  }

  if (_stableState == HIGH && !_longPressFired) {
    if ((millis() - _pressStartTime) >= LONG_PRESS_TIME) {
      _longPressFired = true;
      _clickCount = 0;
      _multiClickWaiting = false;
      longPress = true;
    }
  }

  if (_multiClickWaiting && _stableState == LOW) {
    if ((millis() - _lastReleaseTime) >= MULTI_CLICK_GAP) {
      _multiClickWaiting = false;
      uint8_t clicks = _clickCount;
      _clickCount = 0;
      if (clicks == 1) singleClick = true;
      else if (clicks == 2) doubleClick = true;
      else if (clicks == 3) tripleClick = true;
      else if (clicks == 4) quadClick = true;
    }
  }
}

void clearTouchState() {
  singleClick = false;
  doubleClick = false;
  tripleClick = false;
  quadClick = false;
  longPress = false;
  bool currentRead = buttonPressed();
  _stableState = currentRead;
  _lastReading = currentRead;
  _longPressFired = true;
  _multiClickWaiting = false;
  _clickCount = 0;
}

// -- Display Selection -- SH1106 (1.3" OLED)
#define LCD_SSD1306
#define LCD_HEIGHT 64U
#define LCD_WIDTH 128U
// #define VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS 4

/* Misc. Settings */
#define PLAYER_SAFE_ZONE_WIDTH 32
#define CACTI_RESPAWN_RATE 50
#define GROUND_CACTI_SCROLL_SPEED 3
#define PTERODACTY_SPEED 5
#define PTERODACTY_RESPAWN_RATE 255
#define INCREASE_FPS_EVERY_N_SCORE_POINTS 256
#define LIVES_START 3
#define LIVES_MAX 5
#define SPAWN_NEW_LIVE_MIN_CYCLES 800
#define DAY_NIGHT_SWITCH_CYCLES 1024
#define TARGET_FPS_START 23
#define TARGET_FPS_MAX 48

using namespace TRexGame;

/* Defines and globals */
#define LCD_BYTE_SZIE (LCD_WIDTH * LCD_HEIGHT / 8)
#define LCD_IF_VIRTUAL_WIDTH(TRUE_COND, FALSE_COND) FALSE_COND
#define LCD_PART_BUFF_WIDTH LCD_WIDTH
#define LCD_PART_BUFF_HEIGHT (VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS * 8)
#define LCD_PART_BUFF_SZ ((LCD_PART_BUFF_HEIGHT / 8) * LCD_PART_BUFF_WIDTH)

oledfx lcd(0x3C, size_display::W128xH64, i2c1, 14, 15);

bool dinoFirstRun = true;

/* Misc Functions */
bool isPressedJump() { return buttonPressed(); }

bool isPressedDuck() { return false; }

uint8_t randByte() {
  static uint16_t c = 0xA7E2;
  c = (c << 1) | (c >> 15);
  c = (c << 1) | (c >> 15);
  c = (c << 1) | (c >> 15);
  c ^= static_cast<uint16_t>(get_rand_32());
  return c;
}

/* Main Functions */
struct Renderer {
  static void renderNumber(BitCanvas &canvas, Point2Di8 point,
                           const uint16_t number) {
    uint16_t base = 10000;
    while (base) {
      const uint8_t digit = (number / base) % 10;
      canvas.render(numbers.getSprite(digit, point));
      base /= 10;
      point.x += numbers.getWidth() + 1;
    }
  }
};

void gameLoop() {
  uint8_t lcdBuff[LCD_PART_BUFF_SZ];
  VirtualBitCanvas bitCanvas(
      LCD_IF_VIRTUAL_WIDTH(VirtualBitCanvas::VIRTUAL_WIDTH,
                           VirtualBitCanvas::VIRTUAL_HEIGHT),
      lcdBuff, LCD_PART_BUFF_HEIGHT, LCD_PART_BUFF_WIDTH,
      LCD_IF_VIRTUAL_WIDTH(LCD_WIDTH, LCD_HEIGHT));

  SpawnHold spawnHolder;

  // dynamic sprites
  TrexPlayer trex;
  Ground ground1(-1);
  Ground ground2(63);
  Ground ground3(127);
  Cactus cactus1(spawnHolder);
  Cactus cactus2(spawnHolder);
  Pterodactyl pterodactyl1(spawnHolder);
  HeartLive heartLive;
  const array<SpriteAnimated *, 8> sprites{{&ground1, &ground2, &ground3,
                                             &cactus1, &cactus2, &pterodactyl1,
                                             &heartLive, &trex}};
  const array<SpriteAnimated *, 3> enemies{{&cactus1, &cactus2, &pterodactyl1}};

  // static sprites
  const Sprite gameOverSprite(&game_overver_bm, {15, 12});
  const Sprite restartIconSprite(&restart_icon_bm, {55, 25});
  Sprite heartsSprite(&hearts_5x_bm, {95, 8});

  // game variables
  uint32_t prvT = 0;
  bool gameOver = false;
  uint16_t score = 0;
  uint8_t targetFPS = TARGET_FPS_START;
  uint8_t lives = LIVES_START;
  bool night = false;
  bool jumpSoundPlayed = false;
  lcd.invertColors(night);

  // main cycle
  while (1) {
    // background tasks
    updateTouch();
    buzzer_update();

    // render cycle
    while (1) {
      // score
      Renderer::renderNumber(bitCanvas, {95, 0}, score);
      bitCanvas.render(heartsSprite);
      // game objects
      for (uint8_t i = 0; i < sprites.size(); ++i)
        bitCanvas.render(*sprites[i]);
      // game over
      if (gameOver) {
        bitCanvas.render(gameOverSprite);
        bitCanvas.render(restartIconSprite);
      }
      // update screen
      lcd.display(lcdBuff);
      if (bitCanvas.nextPart())
        break;
    }

    // exit game on game over
    if (gameOver) {
      sleep_ms(500);
      clearTouchState();

      while (1) {
        updateTouch();
        buzzer_update();

        if (singleClick || longPress) {
          singleClick = false;
          longPress = false;
          PLAY(SND_MENU_BACK);
          clearTouchState();
          return;
        }
        serviceUsb();
      }
    }

    // collision detection
    if (!trex.isBlinking() &&
        CollisionDetector::check(trex, enemies.data, enemies.size())) {
      if (lives) {
        trex.blink();
        --lives;
        PLAY(SND_DINO_HIT);
      } else {
        trex.die();
        gameOver = true;
        PLAY(SND_DINO_GAMEOVER);
        continue;
      }
    }
    if (lives < LIVES_MAX && CollisionDetector::check(trex, heartLive)) {
      ++lives;
      heartLive.eat();
      PLAY(SND_DINO_HEART);
    }

    // controls
    if (isPressedJump()) {
      if (!jumpSoundPlayed && trex.state != TrexPlayer::DEAD) {
        PLAY(SND_DINO_JUMP);
        jumpSoundPlayed = true;
      }
      trex.jump();
    } else {
      jumpSoundPlayed = false;
    }
    trex.duck(isPressedDuck());

    // logic and animation step
    for (uint8_t i = 0; i < sprites.size(); ++i)
      sprites[i]->step();
    // score keeping
    if (score < 0xFFFE)
      ++score;
    // make game progressively faster
    if (!(score % INCREASE_FPS_EVERY_N_SCORE_POINTS) &&
        targetFPS < TARGET_FPS_MAX)
      ++targetFPS;
    heartsSprite.limitRenderWidthTo = 6 * lives + 1;
    // switch day and night
    if (!(score % DAY_NIGHT_SWITCH_CYCLES))
      lcd.invertColors(night = !night);

    const uint8_t frameTime = 1000 / targetFPS;

    // throttle
    while (millis() - prvT < frameTime) {
      updateTouch();
      buzzer_update();
  serviceUsb();
    }
    prvT = millis();
  }
}

void spalshScreen() {
  lcd.clear();
  lcd.display();

  while (isPressedJump()) {
    serviceUsb();
  }

  clearTouchState();

  while (1) {
    updateTouch();
    buzzer_update();
    if (singleClick || doubleClick || isPressedJump()) {
      clearTouchState();
      break;
    }
    serviceUsb();
  }
}

void dino_enter() {
  dinoFirstRun = true;
  clearTouchState();
}

void runDinoGame() {
  if (dinoFirstRun) {
    lcd.invertColors(false);
    spalshScreen();
    srand((randByte() << 8) | randByte());
    dinoFirstRun = false;
  }

  gameLoop();
}
}