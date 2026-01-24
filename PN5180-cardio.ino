#include "Config.h"
#include "src/PN5180/PN5180.h"
#include "src/PN5180/PN5180FeliCa.h"
#include "src/PN5180/PN5180ISO15693.h"
#if WITH_ISO14443 == 1
  #include "src/PN5180/PN5180ISO14443.h"
#endif
#include "CARDIOHID.h"
#include <Keypad.h>

PN5180FeliCa nfcFeliCa(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
PN5180ISO15693 nfc15693(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
#if WITH_ISO14443 == 1
  PN5180ISO14443 nfc14443(PN5180_PIN_NSS, PN5180_PIN_BUSY, PN5180_PIN_RST);
#endif

#if WITH_KEYPAD == 1
  /* Keypad declarations */
  const byte ROWS = 4;
  const byte COLS = 3;
  /* This is to use the toprow keys */
  char numpad[ROWS][COLS] = {
   {'7', '8', '9'},
   {'4', '5', '6'},
   {'1', '2', '3'},
   {'0', ',', '\337'}
  };

  byte rowPins[ROWS] = {PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4};
  byte colPins[COLS] = {PIN_COL1, PIN_COL2, PIN_COL3};
  Keypad kpd = Keypad( makeKeymap(numpad), rowPins, colPins, ROWS, COLS );
#endif
 
void setup() {
  Cardio.begin();

#if WITH_KEYPAD == 1
/* Keypad */
    kpd.setDebounceTime(10);
#endif

/* NFC */
  nfcFeliCa.begin();
  nfcFeliCa.reset();

  uint8_t productVersion[2];
  nfcFeliCa.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  if (0xff == productVersion[1]) { // if product version 255, the initialization failed
    exit(-1); // halt
  }
  
  nfcFeliCa.setupRF();
}

unsigned long lastReport = 0;
int cardBusy = 0;

// read cards loop
void loop() {
#if WITH_KEYPAD == 1
  /* KEYPAD */
  keypadCheck();
#endif
  
  /* NFC */
  if (millis()-lastReport < cardBusy) return;
  
  cardBusy = 0;
  uint8_t uid[8] = {0,0,0,0,0,0,0,0};
  uint8_t hid_data[8] = {0,0,0,0,0,0,0,0};
  
  // check for FeliCa card (Suica, Aime, newer Banapassport)
  nfcFeliCa.loadRFConfig(0x09, 0x89); // FeliCa 424
  delay(10); 
  uint8_t uidLength = nfcFeliCa.readCardSerial(uid);
  if (uidLength > 0) {
    Cardio.sendState(2, uid);      
    lastReport = millis();
    cardBusy = 3000;
    return;
  }

  // check for ISO-15693 card (Older Banapassport)
  nfc15693.loadRFConfig(0x0d, 0x8d); // ISO15693
  delay(10);
  ISO15693ErrorCode rc = nfc15693.getInventory(uid);
  if (rc == ISO15693_EC_OK ) {
    for (int i=0; i<8; i++) {
      hid_data[i] = uid[7-i];
    }
    Cardio.sendState(1, hid_data);
    lastReport = millis();
    cardBusy = 3000;
    return;
  }

#if WITH_ISO14443 == 1
  // check for ISO14443 card (Mifare, generic cards)
  nfc14443.loadRFConfig(0x00, 0x80); // ISO14443A
  delay(10);
  uint8_t uidLengthMF = nfc14443.readCardSerial(uid);
  if (uidLengthMF > 0) 
  {
      // If it's a 4-byte UID, repeat it like the PN532 version does
      if (uidLengthMF == 4) {
        for(int i=0; i<4; i++) uid[i+4] = uid[i];
      }
      
      uid[0] &= 0x0F; 
      Cardio.sendState(2, uid);
      lastReport = millis();
      cardBusy = 3000;
      return;
    }
#endif /* ISO14443 */

  // no card detected
  lastReport = millis();
  cardBusy = 200;
}

#if WITH_KEYPAD == 1
void keypadCheck(){
    if (kpd.getKeys())
  {
    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        switch (kpd.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
            Cardio.keyboardPress(kpd.key[i].kchar);
            break;
          case HOLD:
            break;
          case RELEASED:
            Cardio.keyboardRelease(kpd.key[i].kchar);
            break;
          case IDLE:
            break;
        }

      }
    }
  }
}
#endif