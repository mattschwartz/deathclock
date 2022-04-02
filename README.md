# deathclock
Countdown to death. RIP in pieces. Update `birthdate` with your birthdate (add years for old people born before 2000) and `deathdate` with some expected end/death date for tracking.

Supports hours and days display.

## Required Arduino Libraries
- [Adafruit_ST7735_and_ST7789_Library](https://github.com/adafruit/Adafruit-ST7735-Library) for TFT display
- [RTCLib](https://github.com/adafruit/RTClib) for precision RTC

## Components Used
- [Arduino Uno Rev3](https://store-usa.arduino.cc/products/arduino-uno-rev3)
- [Adafruit 1.9" 320x170 Color IPS TFT Display - ST7789](https://www.adafruit.com/product/5394)
- [Adafruit DS3231 Precision RTC Breakout](https://www.adafruit.com/product/3013)

## Compiling and uploading using Arduino CLI:

```
arduino-cli.exe upload .\deathclock.ino -p COM3 -b arduino:avr:uno
arduino-cli.exe compile .\deathclock.ino -b arduino:avr:uno
```