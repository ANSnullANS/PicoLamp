#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "pico/unique_id.h"
#include "hardware/gpio.h"

#include <tusb.h>
#include "Adafruit_NeoPixel.hpp"

#include "config.h"
#include "Rainbow.h"

#ifndef DEBUG
#define DEBUG_printf printf
#endif

// unique board id
char board_id_hex[16];

// Status variables
Color oSelectedColor;
Color oDaylightColor = { 255, 255, 251 };
Color oSignatureColor = { 0, 100, 255 };

int iBrightness = 240;
bool bIsRainbow = true;
bool bIsDaylight = false;
bool bSetColor = false;
bool bIsSignature = false;

static mutex_t oLEDUpdateMutex;

// Timing-Variables
static uint32_t iNow = 0;
static uint32_t iLastStatusUpdate = 0;

Adafruit_NeoPixel oPixels = Adafruit_NeoPixel(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
Rainbow oRainbow;

pico_unique_board_id_t board_id;

LIGHT_MODE CURRENT_MODE = UNDEF; // Currently Active Light-Mode.
LIGHT_MODE DETECTED_MODE = UNDEF; // Light-Mode detected by checking Switch-Status.

void setBrightness(char *cLevel) {
  iBrightness = atoi(cLevel);

  #ifdef DEBUG
  printf("Level: ");
  printf("%d\n", iBrightness);
  #endif

  oPixels.setBrightness(iBrightness);
  oPixels.show();
}

void showStatus(Color oColor) {
  // Use Pixels for Status
  oPixels.clear();
  oPixels.setPixelColor(0, oPixels.Color(oColor.r,oColor.g,oColor.b));
  oPixels.show();
  sleep_ms(1000);
}


void sendUpdate() {
  if(mutex_enter_timeout_ms(&oLEDUpdateMutex, 100))
  {
    for(int i=0; i < NUM_PIXELS; i++) {
      oPixels.setPixelColor(i, oPixels.Color(oRainbow.CHA_Values[i].r, oRainbow.CHA_Values[i].g, oRainbow.CHA_Values[i].b));
    }
    oPixels.show();

    mutex_exit(&oLEDUpdateMutex);
  }
}


void toggle_color() {
  oPixels.clear();
  for (int i=0; i<=NUM_PIXELS; i++) {
    oPixels.setPixelColor(i, oPixels.Color(oSelectedColor.r, oSelectedColor.g, oSelectedColor.b));
  }
  oPixels.show();
}

void core1_entry() {
  // Configure LEDS
  printf("Configuring LEDs...");
  oPixels.begin();
  oPixels.clear();
  oPixels.setBrightness(iBrightness);
  oPixels.show();
  printf("OK\n");
  sleep_ms(100);

  for(;;) {
    iNow = to_ms_since_boot(get_absolute_time());

    if(CURRENT_MODE == RAINBOW) {
      oRainbow.updateMultiplier(iNow);

      oRainbow.calculateValues();
      sendUpdate();

      // Update LED-Positions
      oRainbow.iCurrent = oRainbow.safeIncrement(oRainbow.iCurrent, NUM_PIXELS - 1);

      // Update Rainbow-Positions
      if(oRainbow.iMulti >= RAINBOW_UPDATE_INTERVAL) {
        oRainbow.iRainbow = oRainbow.safeIncrement(oRainbow.iRainbow, RAINBOWMAX - 1);
        oRainbow.iRainbowNext = oRainbow.safeIncrement(oRainbow.iRainbowNext, RAINBOWMAX - 1);

        oRainbow.lLastUpdate = iNow;
      }
    }
    else { // Sleep a bit more when running in Daylight or Signature Mode to save power.
      sleep_ms(300);
    }

    sleep_ms(10);
  }
}


void checkMode() {
  bIsDaylight = gpio_get(DAYLIGHT_PIN);
  bIsRainbow = gpio_get(RAINBOW_PIN);
  if (!bIsDaylight && !bIsRainbow) {
    oSelectedColor = oSignatureColor;
    toggle_color();
    DETECTED_MODE = SIGNATURE;
  }
  else if(bIsDaylight) {
    oSelectedColor = oDaylightColor;
    toggle_color();
    DETECTED_MODE = DAYLIGHT;
  }
  else if(bIsRainbow) {
    DETECTED_MODE = RAINBOW;
  }

  if (DETECTED_MODE != CURRENT_MODE) {
    CURRENT_MODE = DETECTED_MODE;
  }
}


int main() {
    bi_decl(bi_program_description("PicoLamp"));
    bi_decl(bi_1pin_with_name(LED_PIN, "Data-Pin for LED-Stripe."));
    bi_decl(bi_1pin_with_name(NUM_PIXELS, "Amount of LEDs"));
    bi_decl(bi_1pin_with_name(RAINBOW_PIN, "Rainbow-Enable Pin"));
    bi_decl(bi_1pin_with_name(DAYLIGHT_PIN, "Daylight-Enable Pin"));
    bi_decl(bi_program_version_string(VERSION_NO));

    mutex_init(&oLEDUpdateMutex);

    stdio_init_all();

    pico_get_unique_board_id(&board_id);
    printf("\nUnique identifier:");
    for (int i = 0; i < PICO_UNIQUE_BOARD_ID_SIZE_BYTES; ++i) {
        printf(" %02x", board_id.id[i]);
    }
    printf("\n");

    // Configure Pins
    gpio_init(RAINBOW_PIN);
    gpio_set_dir(RAINBOW_PIN, GPIO_IN);
    gpio_init(DAYLIGHT_PIN);
    gpio_set_dir(DAYLIGHT_PIN, GPIO_IN);

    
    // Check Selected Light-Mode
    checkMode();
    
    // Launch LED-Processing on 2nd Core.
    multicore_launch_core1(core1_entry);

    // Run Pin-Checks on 1st Core.
    while(true) {
      // Handle Mode-Changes
      checkMode();

      sleep_ms(300);
    }


    return 0;
};