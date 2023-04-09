#ifndef CONFIG_H
#define CONFIG_H

#include "pico/stdlib.h"
#include "pico/stdio.h"

#define VERSION_NO "1.0.0"

#define DEBUG true
//#define DEBUG_RAINBOW true

#define LED_PIN 2
#define NUM_PIXELS 24

#define DAYLIGHT_PIN  15
#define RAINBOW_PIN  14

#define RAINBOW_UPDATE_INTERVAL 30000 // Interval in milliseconds

#ifndef COLOR_STRUCT
#define COLOR_STRUCT
struct Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};
#endif

#ifndef LIGHT_ENUMS
#define LIGHT_ENUMS
enum LIGHT_MODE {
  UNDEF,
  DAYLIGHT,
  RAINBOW,
  SIGNATURE
};
#endif


#endif // CONFIG_H