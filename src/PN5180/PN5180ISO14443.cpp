#include <Arduino.h>
#include "PN5180ISO14443.h"
#include "PN5180.h"
#include "Debug.h"

PN5180ISO14443::PN5180ISO14443(uint8_t SSpin, uint8_t BUSYpin, uint8_t RSTpin) 
              : PN5180(SSpin, BUSYpin, RSTpin) {
}

bool PN5180ISO14443::setupRF() {
  if (!loadRFConfig(0x00, 0x80)) return false;
  if (!setRF_on()) return false;
  return true;
}

uint16_t PN5180ISO14443::rxBytesReceived() {
	uint32_t rxStatus = 0;
	readRegister(RX_STATUS, &rxStatus);
	return (uint16_t)(rxStatus & 0x000001ff);
}

uint8_t PN5180ISO14443::activateTypeA(uint8_t *buffer, uint8_t kind) {
	uint8_t cmd[7];
	
	writeRegisterWithAndMask(SYSTEM_CONFIG, 0xFFFFFFBF); // OFF Crypto
	writeRegisterWithAndMask(CRC_RX_CONFIG, 0xFFFFFFFE); // Clear RX CRC
	writeRegisterWithAndMask(CRC_TX_CONFIG, 0xFFFFFFFE); // Clear TX CRC
    
    clearIRQStatus(0xFFFFFFFF);
	cmd[0] = (kind == 0) ? 0x26 : 0x52; // REQA/WUPA
	if (!sendData(cmd, 1, 0x07)) return 0;
    
    // Wait for RX
    unsigned long start = millis();
    while (0 == (RX_IRQ_STAT & getIRQStatus())) {
        if (millis() - start > 50) return 0;
    }

	if (!readData(2, buffer)) return 0;
    
	cmd[0] = 0x93; // Anti collision 1
	cmd[1] = 0x20;
    clearIRQStatus(0xFFFFFFFF);
	if (!sendData(cmd, 2, 0x00)) return 0;
    
    start = millis();
    while (0 == (RX_IRQ_STAT & getIRQStatus())) {
        if (millis() - start > 50) return 0;
    }
    
	if (!readData(5, cmd+2)) return 0;
    
	writeRegisterWithOrMask(CRC_RX_CONFIG, 0x01); 
	writeRegisterWithOrMask(CRC_TX_CONFIG, 0x01); 
    
	cmd[0] = 0x93; // Select
	cmd[1] = 0x70;
    clearIRQStatus(0xFFFFFFFF);
	if (!sendData(cmd, 7, 0x00)) return 0;
    
    start = millis();
    while (0 == (RX_IRQ_STAT & getIRQStatus())) {
        if (millis() - start > 50) return 0;
    }
    
	if (!readData(1, buffer+2)) return 0;
    
	if ((buffer[2] & 0x04) == 0) { // 4 Byte UID
		for (int i = 0; i < 4; i++) buffer[3+i] = cmd[2 + i];
		return 4;
	}
    
    // 7 Byte UID support... (simplified for now to match original flow)
    return 7; 
}

uint8_t PN5180ISO14443::readCardSerial(uint8_t *buffer) {
    uint8_t response[10];
    memset(response, 0, 10);
    uint8_t uidLength = activateTypeA(response, 1);
	if (uidLength == 0) return 0;
	if ((response[3] == 0x00) && (response[4] == 0x00) && (response[5] == 0x00) && (response[6] == 0x00)) return 0;
    
    for (int i = 0; i < 7; i++) buffer[i] = response[i+3];
	mifareHalt();
	return uidLength;  
}

bool PN5180ISO14443::mifareHalt() {
	uint8_t cmd[2] = {0x50, 0x00};
	sendData(cmd, 2, 0x00);	
	return true;
}

bool PN5180ISO14443::isCardPresent() {
    uint8_t buffer[10];
	return (readCardSerial(buffer) >=4);
}