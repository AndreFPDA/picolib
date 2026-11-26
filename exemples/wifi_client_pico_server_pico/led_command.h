#ifndef LED_COMMAND_H
#define LED_COMMAND_H

#include <ctype.h>
#include <stddef.h>

typedef enum {
    LED_COMMAND_INVALID,
    LED_COMMAND_ON,
    LED_COMMAND_OFF,
    LED_COMMAND_TOGGLE
} led_command_t;

static inline led_command_t led_command_parse(const char *line) {
    char command[16];
    size_t length = 0;

    while (*line != '\0') {
        unsigned char character = (unsigned char)*line++;
        if (isspace(character)) {
            continue;
        }
        if (length >= sizeof(command) - 1) {
            return LED_COMMAND_INVALID;
        }
        command[length++] = (char)toupper(character);
    }
    command[length] = '\0';

    if (length == 2 && command[0] == 'O' && command[1] == 'N') {
        return LED_COMMAND_ON;
    }
    if (length == 3 && command[0] == 'O' && command[1] == 'F' && command[2] == 'F') {
        return LED_COMMAND_OFF;
    }
    if (length == 6 && command[0] == 'T' && command[1] == 'O' &&
        command[2] == 'G' && command[3] == 'G' && command[4] == 'L' &&
        command[5] == 'E') {
        return LED_COMMAND_TOGGLE;
    }
    if (length == 5 && command[0] == 'L' && command[1] == 'E' &&
        command[2] == 'D' && command[3] == 'O' && command[4] == 'N') {
        return LED_COMMAND_ON;
    }
    if (length == 6 && command[0] == 'L' && command[1] == 'E' &&
        command[2] == 'D' && command[3] == 'O' && command[4] == 'F' &&
        command[5] == 'F') {
        return LED_COMMAND_OFF;
    }
    if (length == 9 && command[0] == 'L' && command[1] == 'E' &&
        command[2] == 'D' && command[3] == 'T' && command[4] == 'O' &&
        command[5] == 'G' && command[6] == 'G' && command[7] == 'L' &&
        command[8] == 'E') {
        return LED_COMMAND_TOGGLE;
    }
    return LED_COMMAND_INVALID;
}

static inline const char *led_command_text(led_command_t command) {
    switch (command) {
        case LED_COMMAND_ON: return "ON";
        case LED_COMMAND_OFF: return "OFF";
        case LED_COMMAND_TOGGLE: return "TOGGLE";
        default: return NULL;
    }
}

#endif
