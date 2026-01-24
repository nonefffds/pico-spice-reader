#include <Arduino.h>
#include "PN5180FeliCa.h"
#include "PN5180.h"
#include "Debug.h"

PN5180FeliCa::PN5180FeliCa(uint8_t SSpin, uint8_t BUSYpin, uint8_t RSTpin) 
              : PN5180(SSpin, BUSYpin, RSTpin) {
}

bool PN5180FeliCa::setupRF() {
  if (!loadRFConfig(0x09, 0x89)) return false;
  if (!setRF_on()) return false;
  return true;
}

uint8_t PN5180FeliCa::pol_req(uint8_t *buffer) {
	uint8_t cmd[6];
	// OFF Crypto
	writeRegisterWithAndMask(SYSTEM_CONFIG, 0xFFFFFFBF);
    
    cmd[0] = 0x06;             // total length
	cmd[1] = 0x00;             // POL_REQ command
	cmd[2] = 0xFF;             
	cmd[3] = 0xFF;             // any target
	cmd[4] = 0x01;             // System Code request
	cmd[5] = 0x00;             // 1 timeslot only
    
    clearIRQStatus(0xFFFFFFFF);
	if (!sendData(cmd, 6, 0x00)) return 0;
    
    // Wait for RX IRQ
    unsigned long start = millis();
    while (0 == (RX_IRQ_STAT & getIRQStatus())) {
        if (millis() - start > 100) return 0; // Timeout
    }
   
	if (!readData(20, buffer)) return 0;
    
    // check Response Code
    if (buffer[1] != 0x01) return 0;
    
    return 8; // IDm length
}

uint8_t PN5180FeliCa::readCardSerial(uint8_t *buffer) {
    uint8_t response[20];
    memset(response, 0, 20);
    
    uint8_t uidLength = pol_req(response);
    if (uidLength == 0) return 0;
  
    for (int i = 0; i < uidLength; i++) 
        buffer[i] = response[i+2];
    
	return uidLength;  
}

bool PN5180FeliCa::isCardPresent() {
    uint8_t buffer[8];
	return (readCardSerial(buffer) != 0);
}