/*
 * Project name: T-rex-duino
 * Description: T-rex game from Chrome browser rewritten for Arduino
 * Project page: https://github.com/AlexIII/t-rex-duino
 * Author: github.com/AlexIII
 * Standalone adaptation for ESP01 / Arduino
 */

#ifndef T_REX_DUINO_H
#define T_REX_DUINO_H

#include <stdint.h>

namespace t_rex
{
struct BuzzerNote
{
    int freq;
    int dur;
};

extern const BuzzerNote SND_MENU_BACK[];
extern const BuzzerNote SND_DINO_JUMP[];
extern const BuzzerNote SND_DINO_HIT[];
extern const BuzzerNote SND_DINO_GAMEOVER[];
extern const BuzzerNote SND_DINO_HEART[];

void buzzer_update();
void buzzer_play(const BuzzerNote *seq, int len);

bool buttonPressed();
void updateTouch();
void clearTouchState();

extern bool singleClick;
extern bool doubleClick;
extern bool tripleClick;
extern bool quadClick;
extern bool longPress;

bool isPressedJump();
bool isPressedDuck();
uint8_t randByte();
void dino_enter();
void runDinoGame();
}

#endif
