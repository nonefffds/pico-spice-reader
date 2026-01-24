# Porting to Raspberry Pi Pico (arduino-pico)

This project has been ported to work with the Raspberry Pi Pico using the [earlephilhower/arduino-pico](https://github.com/earlephilhower/arduino-pico) core and the **Adafruit TinyUSB** library.

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

## Pinout

The following pins are used on the Raspberry Pi Pico:

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

### Matrix Keypad (Optional)
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
The project is configured to use VID `0x1ccf` and PID `0x5252` (Sega/Amusement device style). If you need to reflash the device, you may need to put the Pico into BOOTSEL mode manually if the custom VID/PID prevents the IDE from auto-resetting.

## Changes made for porting:
- Replaced the AVR/SAM-specific `PluggableUSB` code with `Adafruit_TinyUSB`.
- Updated `CARDIOHID` class to use `Adafruit_USBD_HID`.
- Updated pin definitions in `Config.h` to match Pico's GPIO layout.
- Added `Cardio.begin()` to the main sketch to initialize the TinyUSB stack.
