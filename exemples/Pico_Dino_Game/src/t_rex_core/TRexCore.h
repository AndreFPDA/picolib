#ifndef TREX_CORE_H
#define TREX_CORE_H

#include <stdint.h>
#include "../../../../lib/oledfx/oledfx.hpp"
#include "../../../../lib/oledfx/OLED_font.hpp"
#include "../bit_map_img.hpp"

#define LCD_SSD1306
#define LCD_HEIGHT 64U
#define LCD_WIDTH 128U
#define VIRTUAL_HEIGHT_BUFFER_ROWS_BY_8_PIXELS 8
#define PLAYER_SAFE_ZONE_WIDTH 32
#define CACTI_RESPAWN_RATE 50
#define GROUND_CACTI_SCROLL_SPEED 3
#define PTERODACTY_SPEED 5
#define PTERODACTY_RESPAWN_RATE 255
#define SPAWN_NEW_LIVE_MIN_CYCLES 800

#include "core_types.hpp"
#include "assets.hpp"
#include "trex_player.hpp"
#include "pterodactyl.hpp"
#include "ground.hpp"
#include "heart_live.hpp"
#include "cactus.hpp"

#endif
