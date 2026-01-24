#ifndef CONFIG_h
#define CONFIG_h

/* ISO14443 support (for older Aime/Nesica/BANAPASSPORT cards... reader will pretend it was a FeliCa for maximum cardio compatibility) */
#define WITH_ISO14443 1

/* Pinout for the PN5180 free pins on Raspberry Pi Pico */
/* SPI0: SCK=GP18, MOSI=GP19, MISO=GP16 */
#define PN5180_PIN_NSS  17
#define PN5180_PIN_BUSY 20
#define PN5180_PIN_RST  21

/* Use a matrix keypad */
#define WITH_KEYPAD 0
  #define PIN_ROW1 0
  #define PIN_ROW2 1
  #define PIN_ROW3 2
  #define PIN_ROW4 3
  #define PIN_COL1 4
  #define PIN_COL2 5
  #define PIN_COL3 6

/* Player ID (1 or 2) */
#define CARDIO_ID 1
/* Enable custom VID/PID (handled by TinyUSB) */
#define CUSTOM_VIDPID 1

#endif