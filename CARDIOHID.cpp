#include "Config.h"
#include "CARDIOHID.h"

// Report IDs
#define REPORT_ID_CARDIO_1  1
#define REPORT_ID_CARDIO_2  2
#define REPORT_ID_KEYBOARD  3

static const uint8_t hid_report[] = {
    // Custom Cardio Reports
    0x06, 0xCA, 0xFF,  // Usage Page (Vendor Defined 0xFFCA)
    0x09, 0x01,        // Usage (0x01)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_CARDIO_1,        //   Report ID (1)
    0x06, 0xCA, 0xFF,  //   Usage Page (Vendor Defined 0xFFCA)
    0x09, 0x41,        //   Usage (0x41)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0xFF,        //   Logical Maximum (-1)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    
    0x85, REPORT_ID_CARDIO_2,        //   Report ID (2)
    0x06, 0xCA, 0xFF,  //   Usage Page (Vendor Defined 0xFFCA)
    0x09, 0x42,        //   Usage (0x42)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0xFF,        //   Logical Maximum (-1)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection

    // Standard Keyboard Report
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x06,        // Usage (Keyboard)
    0xA1, 0x01,        // Collection (Application)
    0x85, REPORT_ID_KEYBOARD, // Report ID (3)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0xE0,        //   Usage Minimum (0xE0)
    0x29, 0xE7,        //   Usage Maximum (0xE7)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x08,        //   Report Count (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x05,        //   Report Count (5)
    0x75, 0x01,        //   Report Size (1)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x05,        //   Usage Maximum (0x05)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x75, 0x03,        //   Report Size (3)
    0x91, 0x01,        //   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x06,        //   Report Count (6)
    0x75, 0x08,        //   Report Size (8)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x65,        //   Logical Maximum (101)
    0x05, 0x07,        //   Usage Page (Kbrd/Keypad)
    0x19, 0x00,        //   Usage Minimum (0x00)
    0x29, 0x65,        //   Usage Maximum (0x65)
    0x81, 0x00,        //   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0               // End Collection
};

CARDIOHID_::CARDIOHID_(void) {
    memset(_key_report, 0, 6);
}

bool CARDIOHID_::begin(void) {
#if CUSTOM_VIDPID == 1
    USBDevice.setID(0x1ccf, 0x5252);
#endif
    USBDevice.setManufacturerDescriptor("CrazyRedMachine");
    USBDevice.setProductDescriptor("CardIO");
#if CARDIO_ID == 1
    USBDevice.setSerialDescriptor("CARDIOP1");
#else
    USBDevice.setSerialDescriptor("CARDIOP2");
#endif

    usb_hid.setPollInterval(2);
    usb_hid.setReportDescriptor(hid_report, sizeof(hid_report));
    
    return usb_hid.begin();
}

int CARDIOHID_::sendState(uint8_t type, uint8_t *value) {
    if (!usb_hid.ready()) return 0;
    
    // The original code sent 9 bytes: [report_id, 8 bytes of UID]
    // In TinyUSB, sendReport(id, data, len) sends [id, data...]
    // If the original hid_report counts 8 bytes for the payload, 
    // we should send exactly 8 bytes of data, and TinyUSB adds the ID.
    // However, the original code's sendState was:
    // data[0] = type; memcpy(data+1, value, 8); return USB_Send(..., data, 9);
    // And their descriptor had 95, 08 (8 bytes) per report.
    // So the total packet on wire was 9 bytes (1 ID + 8 Data).
    
    return usb_hid.sendReport(type, value, 8) ? 9 : 0;
}

void CARDIOHID_::keyboardPress(char c) {
    uint8_t keycode = _asciiToKeycode(c);
    if (keycode == 0) return;

    for (int i = 0; i < 6; i++) {
        if (_key_report[i] == keycode) return; // Already pressed
        if (_key_report[i] == 0) {
            _key_report[i] = keycode;
            _sendKeyboardReport();
            return;
        }
    }
}

void CARDIOHID_::keyboardRelease(char c) {
    uint8_t keycode = _asciiToKeycode(c);
    if (keycode == 0) return;

    for (int i = 0; i < 6; i++) {
        if (_key_report[i] == keycode) {
            _key_report[i] = 0;
            // Shift remaining keys
            for (int j = i; j < 5; j++) {
                _key_report[j] = _key_report[j+1];
            }
            _key_report[5] = 0;
            _sendKeyboardReport();
            return;
        }
    }
}

void CARDIOHID_::_sendKeyboardReport() {
    if (!usb_hid.ready()) return;
    
    uint8_t report[8] = { 0 };
    // report[0] is modifiers, leave at 0 for now as keypad doesn't use them
    // report[1] is reserved
    memcpy(&report[2], _key_report, 6);
    
    usb_hid.sendReport(REPORT_ID_KEYBOARD, report, 8);
}

uint8_t CARDIOHID_::_asciiToKeycode(char c) {
    if (c >= '1' && c <= '9') return 0x1E + (c - '1');
    if (c == '0') return 0x27;
    if (c == ',') return 0x36; // Comma
    if (c == '\337') return 0x29; // Escape (used for test/service usually in these readers)
    return 0;
}

CARDIOHID_ Cardio;
