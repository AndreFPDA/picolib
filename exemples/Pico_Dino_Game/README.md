# ESP32 Chrome Dino Game

A Chrome-Dino-style runner game built on an ESP32, an SSD1306 OLED display,
a push button, and a buzzer.

## Connect with TechTadka360

- YouTube: [@techtadka360official](https://youtube.com/@techtadka360official?si=GdlIntZKv30kPgBk)
- Instagram: [@techtadka360official](https://www.instagram.com/techtadka360official?igsh=cWR4bnhjdWw1MHdh)
- Facebook: [TechTadka360](https://www.facebook.com/share/1EkKAJNLdB/)

## Hardware Used

| Component        | Notes                        |
|-------------------|-------------------------------|
| ESP32 Dev Board   | Any ESP32 dev board            |
| SSD1306 OLED      | 128x64, I2C                   |
| Push button       | 1 x momentary switch           |
| Buzzer            | Passive buzzer (uses `tone()`) |

## Wiring / Circuit Diagram

```
                         ESP32
                     ┌─────────────┐
        OLED SDA ----│ GPIO21      │
        OLED SCL ----│ GPIO22      │
        OLED VCC ----│ 3V3         │
        OLED GND ----│ GND         │
                     │             │
     Button pin1 ----│ GPIO5       │
     Button pin2 ----│ GND         │
                     │             │
      Buzzer (+) ----│ GPIO12      │
      Buzzer (-) ----│ GND         │
                     └─────────────┘
```

| Signal          | ESP32 Pin | Other end        |
|------------------|-----------|-------------------|
| OLED SDA         | GPIO21    | OLED SDA          |
| OLED SCL         | GPIO22    | OLED SCK/SCL      |
| OLED VCC         | 3V3       | OLED VCC          |
| OLED GND         | GND       | OLED GND          |
| Push button pin 1| GPIO5     | Button terminal A |
| Push button pin 2| GND       | Button terminal B |
| Buzzer +         | GPIO12    | Buzzer positive   |
| Buzzer -         | GND       | Buzzer negative   |

**Button logic:** the sketch uses `INPUT_PULLUP` on GPIO5, so the button
reads `HIGH` when not pressed and `LOW` when pressed (no external resistor
needed).

## Software Setup

1. Install the **Arduino IDE** and the **ESP32 board package**
   (Boards Manager → search "esp32" → install).
2. Install these libraries via **Library Manager**:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
3. Open `dino_esp32.ino`, select your ESP32 board and COM port.
4. Upload.

## How to Play

- Power on: the start screen appears.
- Press the button to start the run.
- Press the button to jump over the incoming cactus.
- Speed increases as your score climbs.
- On collision, a game-over screen and buzzer tone play — press the
  button again to retry.

## Project Structure

```
dino-esp32/
├── dino_esp32.ino   # Main sketch (game logic + hardware I/O)
└── README.md         # This file
```

## Customization Ideas

- Change `JUMP_VELOCITY` / `GRAVITY` in the sketch to tune jump feel.
- Adjust `gameSpeed` increment to change difficulty ramp.
- Swap the buzzer tones in `jumpSound()` / `gameOverSound()` for
  different sound effects.

## Connect with TechTadka360

- YouTube: [@techtadka360official](https://youtube.com/@techtadka360official?si=GdlIntZKv30kPgBk)
- Instagram: [@techtadka360official](https://www.instagram.com/techtadka360official?igsh=cWR4bnhjdWw1MHdh)
- Facebook: [TechTadka360](https://www.facebook.com/share/1EkKAJNLdB/)
