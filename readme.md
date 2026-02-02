# pico-spice-reader

This project is a ported PN5180-cardio to work with the Raspberry Pi Pico using the [earlephilhower/arduino-pico](https://github.com/earlephilhower/arduino-pico) core and the **Adafruit TinyUSB** library.

**Currently in development, possibly not properly working. This is a Proof-of-concept only.**

Huge thanks to the Original Repository: https://github.com/CrazyRedMachine/PN5180-cardio

Also thanks for the inspiration from: https://github.com/whowechina/aic_pico, I also used this project only for cross-testing; there's no code referencing from aic_pico.

This repository includes AIGC contents.

Please notice that there's no plan to make a PN532 port.

## Acknowledgment from original repository

This work is based on zyp's cardio (obviously).

ISO15693 code is based on [ATrappmann/PN5180-Library](https://github.com/ATrappmann/PN5180-Library).

ISO14443 implementation taken from [tueddy/PN5180-Library/ISO14443](https://github.com/tueddy/PN5180-Library/tree/ISO14443).

The keypad code uses the Keypad library by Mark Stanley and Alexander Brevig.

Spiceapi version provided by [goat](https://github.com/goaaats) (thanks! :)), refer to SpiceAPI branch for more information.

## Requirements

1.  **Arduino IDE** with the Raspberry Pi Pico core by Earle F. Philhower, III installed.
2.  **Adafruit TinyUSB Library** (install via Arduino Library Manager).
3.  **Keypad Library** (if using the keypad).

## IDE Settings

When compiling for Raspberry Pi Pico, ensure the following settings are selected in the **Tools** menu:
- **Board:** "Raspberry Pi Pico" (or Pico W)
- **USB Stack:** "Adafruit TinyUSB"
- **Flash Size:** (any)
- **CPU Speed:** (default)

## Usage

As long as you follow the pinout below, you can drag-and-drop the uf2 file from the build folder to the RPi.

## Pinout

The following pins are used on the Raspberry Pi Pico:

You might not need a level converter 3v3<->5v like the original approach. Currently, this pinout is tested and working.

### PN5180 Connection (SPI0)
| PN5180 Pin | Pico Pin (GP) | Pico Physical Pin |
| :--- | :--- | :--- |
| **3.3V** | 3V3 | 36 |
| **GND** | GND | (Any GND) |
| **NSS** | GP17 | 22 |
| **SCK** | GP18 | 24 |
| **MOSI** | GP19 | 25 |
| **MISO** | GP16 | 21 |
| **BUSY** | GP20 | 26 |
| **RST** | GP21 | 27 |

### Matrix Keypad (not tested)

Currently the Keypad is not tested working and disabled by default. This will remain TO-DO now.

| Keypad | Pico Pin (GP) | Pico Physical Pin |
| :--- | :--- | :--- |
| **Row 1** | GP0 | 1 |
| **Row 2** | GP1 | 2 |
| **Row 3** | GP2 | 4 |
| **Row 4** | GP3 | 5 |
| **Col 1** | GP4 | 6 |
| **Col 2** | GP5 | 7 |
| **Col 3** | GP6 | 9 |

## Custom VID/PID
The project is configured to use VID `0x1ccf` and PID `0x5252`. If you need to reflash the device, you may need to put the Pico into BOOTSEL mode manually if the custom VID/PID prevents the IDE from auto-resetting.

## Changes made for porting:
- Replaced the AVR/SAM-specific `PluggableUSB` code with `Adafruit_TinyUSB`.
- Updated `CARDIOHID` class to use `Adafruit_USBD_HID`.
- Updated pin definitions in `Config.h` to match Pico's GPIO layout.
- Added `Cardio.begin()` to the main sketch to initialize the TinyUSB stack.
- Updated PN5180 library to the latest.

## Known Issue

When using Suica cards on iPhone devices, cards will be recognised as ISO14443 cards instead of felica, which will leading a card number difference problem. Osaifu keidai is tested on Kyocera KYF37, which is not affected. 

This behavior is replicable in both this repository and aic_pico repo. 

Since reading ISO14443 cards will give a hard-encoding E004 card number, it's possible to lead to a wrong card number read-out. However, I have tested with the cards I have, and they're working. If there's any problem with your ISO14443 cards, please leave an issue.

### Problem Address

``All of the modules i've purchased had a fatal flaw - they have an invalid clock crystal installed on them - 27.00 MHZ instead of 27.12 MHZ.
It is not a big problem for a passive card that will operate at the freqency given by the reader, but a total dealbreaker for a battery-powered device like an iPhone or Apple Watch, which will de-synchronize after a couple of bytes, thus causing collision/protocol/parity errors.

The only way of making this module compatible is by purchasing a batch of SMD 27.12 MHZ crystals on your own, de-soldering the incorrect one and re-soldering the required one.``

Referenced from https://github.com/kormax/apple-enhanced-contactless-polling/blob/main/examples/README.md

The problem may be caused by the faulty crystal, and I have checked my nfc modules, it's indeed a 27Mhz one. 

## License

GPL-3.0
