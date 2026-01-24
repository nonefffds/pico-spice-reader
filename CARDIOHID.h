#ifndef CARDIOHID_h
#define CARDIOHID_h

#include <Adafruit_TinyUSB.h>

class CARDIOHID_ {
    public:
        CARDIOHID_(void);
        bool begin(void);
        int sendState(uint8_t type, uint8_t *value);
        
        // Manual keyboard implementation
        void keyboardPress(char c);
        void keyboardRelease(char c);
        
    private:
        Adafruit_USBD_HID usb_hid;
        uint8_t _key_report[6]; // Support up to 6 keys
        void _sendKeyboardReport();
        uint8_t _asciiToKeycode(char c);
};

extern CARDIOHID_ Cardio;

#endif