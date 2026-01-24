#include <Arduino.h>
#include "PN5180.h"
#include "Debug.h"

#define PN5180_WRITE_REGISTER           (0x00)
#define PN5180_WRITE_REGISTER_OR_MASK   (0x01)
#define PN5180_WRITE_REGISTER_AND_MASK  (0x02)
#define PN5180_READ_REGISTER            (0x04)
#define PN5180_WRITE_EEPROM				(0x06)
#define PN5180_READ_EEPROM              (0x07)
#define PN5180_SEND_DATA                (0x09)
#define PN5180_READ_DATA                (0x0A)
#define PN5180_SWITCH_MODE              (0x0B)
#define PN5180_LOAD_RF_CONFIG           (0x11)
#define PN5180_RF_ON                    (0x16)
#define PN5180_RF_OFF                   (0x17)

uint8_t PN5180::readBuffer[508];

PN5180::PN5180(uint8_t SSpin, uint8_t BUSYpin, uint8_t RSTpin) {
  PN5180_NSS = SSpin;
  PN5180_BUSY = BUSYpin;
  PN5180_RST = RSTpin;
  // Reduced to 1MHz for maximum stability on RP2040
  PN5180_SPI_SETTINGS = SPISettings(1000000, MSBFIRST, SPI_MODE0);
}

void PN5180::begin() {
  pinMode(PN5180_NSS, OUTPUT);
  pinMode(PN5180_BUSY, INPUT);
  pinMode(PN5180_RST, OUTPUT);

  digitalWrite(PN5180_NSS, HIGH);
  digitalWrite(PN5180_RST, HIGH);

  SPI.setRX(16);
  SPI.setSCK(18);
  SPI.setTX(19);
  SPI.begin();
}

void PN5180::end() {
  digitalWrite(PN5180_NSS, HIGH);
  SPI.end();
}

bool PN5180::writeRegister(uint8_t reg, uint32_t value) {
  uint8_t *p = (uint8_t*)&value;
  uint8_t buf[6] = { PN5180_WRITE_REGISTER, reg, p[0], p[1], p[2], p[3] };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool res = transceiveCommand(buf, 6);
  SPI.endTransaction();
  return res;
}

bool PN5180::writeRegisterWithOrMask(uint8_t reg, uint32_t mask) {
  uint8_t *p = (uint8_t*)&mask;
  uint8_t buf[6] = { PN5180_WRITE_REGISTER_OR_MASK, reg, p[0], p[1], p[2], p[3] };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool res = transceiveCommand(buf, 6);
  SPI.endTransaction();
  return res;
}

bool PN5180::writeRegisterWithAndMask(uint8_t reg, uint32_t mask) {
  uint8_t *p = (uint8_t*)&mask;
  uint8_t buf[6] = { PN5180_WRITE_REGISTER_AND_MASK, reg, p[0], p[1], p[2], p[3] };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool res = transceiveCommand(buf, 6);
  SPI.endTransaction();
  return res;
}

bool PN5180::readRegister(uint8_t reg, uint32_t *value) {
  uint8_t cmd[2] = { PN5180_READ_REGISTER, reg };
  *value = 0; // Clear before read
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool res = transceiveCommand(cmd, 2, (uint8_t*)value, 4);
  SPI.endTransaction();
  return res;
}

bool PN5180::writeEEprom(uint8_t addr, uint8_t *buffer, uint8_t len) {
	uint8_t cmd[len + 2];
	cmd[0] = PN5180_WRITE_EEPROM;
	cmd[1] = addr;
	for (int i = 0; i < len; i++) cmd[2 + i] = buffer[i];
	SPI.beginTransaction(PN5180_SPI_SETTINGS);
	bool res = transceiveCommand(cmd, len + 2);
	SPI.endTransaction();
	return res;
}

bool PN5180::readEEprom(uint8_t addr, uint8_t *buffer, uint8_t len) {
  uint8_t cmd[3] = { PN5180_READ_EEPROM, addr, len };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool res = transceiveCommand(cmd, 3, buffer, len);
  SPI.endTransaction();
  return res;
}

bool PN5180::sendData(uint8_t *data, int len, uint8_t validBits) {
  uint8_t buffer[len+2];
  buffer[0] = PN5180_SEND_DATA;
  buffer[1] = validBits;
  for (int i=0; i<len; i++) buffer[2+i] = data[i];

  writeRegisterWithAndMask(SYSTEM_CONFIG, 0xfffffff8);
  writeRegisterWithOrMask(SYSTEM_CONFIG, 0x00000003);

  PN5180TransceiveStat transceiveState = getTransceiveState();
  if (PN5180_TS_WaitTransmit != transceiveState) return false;

  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool success = transceiveCommand(buffer, len+2);
  SPI.endTransaction();
  return success;
}

uint8_t * PN5180::readData(int len) {
  uint8_t cmd[2] = { PN5180_READ_DATA, 0x00 };
  memset(readBuffer, 0, sizeof(readBuffer));
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  transceiveCommand(cmd, 2, readBuffer, len);
  SPI.endTransaction();
  return readBuffer;
}

bool PN5180::readData(uint8_t len, uint8_t *buffer) {
	uint8_t cmd[2] = { PN5180_READ_DATA, 0x00 };
	SPI.beginTransaction(PN5180_SPI_SETTINGS);
	bool success = transceiveCommand(cmd, 2, buffer, len);
	SPI.endTransaction();
	return success;
}

bool PN5180::switchToLPCD(uint16_t wakeupCounterInMs) {
  clearIRQStatus(0xffffffff); 
  writeRegister(IRQ_ENABLE, LPCD_IRQ_STAT | GENERAL_ERROR_IRQ_STAT);  
  uint8_t cmd[4] = { PN5180_SWITCH_MODE, 0x01, (uint8_t)(wakeupCounterInMs & 0xFF), (uint8_t)((wakeupCounterInMs >> 8U) & 0xFF) };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool success = transceiveCommand(cmd, sizeof(cmd));
  SPI.endTransaction();
  return success;
}

bool PN5180::loadRFConfig(uint8_t txConf, uint8_t rxConf) {
  uint8_t cmd[3] = { PN5180_LOAD_RF_CONFIG, txConf, rxConf };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  bool res = transceiveCommand(cmd, 3);
  SPI.endTransaction();
  return res;
}

bool PN5180::setRF_on() {
  uint8_t cmd[2] = { PN5180_RF_ON, 0x00 };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  transceiveCommand(cmd, 2);
  SPI.endTransaction();
  
  unsigned long start = millis();
  while (0 == (TX_RFON_IRQ_STAT & getIRQStatus())) {
    if (millis() - start > 100) return false;
  }
  clearIRQStatus(TX_RFON_IRQ_STAT);
  return true;
}

bool PN5180::setRF_off() {
  uint8_t cmd[2] { PN5180_RF_OFF, 0x00 };
  SPI.beginTransaction(PN5180_SPI_SETTINGS);
  transceiveCommand(cmd, 2);
  SPI.endTransaction();

  unsigned long start = millis();
  while (0 == (TX_RFOFF_IRQ_STAT & getIRQStatus())) {
    if (millis() - start > 100) return false;
  }
  clearIRQStatus(TX_RFOFF_IRQ_STAT);
  return true;
}

bool PN5180::transceiveCommand(uint8_t *sendBuffer, size_t sendBufferLen, uint8_t *recvBuffer, size_t recvBufferLen) {
  unsigned long startedWaiting = millis();
  while (LOW != digitalRead(PN5180_BUSY)) {
    if (millis() - startedWaiting > commandTimeout) return false;
  }
  
  digitalWrite(PN5180_NSS, LOW);
  delayMicroseconds(50); // Generous margin for Pico
  
  for (uint8_t i=0; i<sendBufferLen; i++) {
    SPI.transfer(sendBuffer[i]);
  }
  
  startedWaiting = millis();
  while (HIGH != digitalRead(PN5180_BUSY)) {
    if (millis() - startedWaiting > commandTimeout) return false;
  }
  
  digitalWrite(PN5180_NSS, HIGH);
  delayMicroseconds(50);
  
  startedWaiting = millis();
  while (LOW != digitalRead(PN5180_BUSY)) {
    if (millis() - startedWaiting > commandTimeout) return false;
  }

  if ((0 == recvBuffer) || (0 == recvBufferLen)) return true;

  delayMicroseconds(50);
  digitalWrite(PN5180_NSS, LOW);
  delayMicroseconds(50);
  
  for (uint8_t i=0; i<recvBufferLen; i++) {
    recvBuffer[i] = SPI.transfer(0xff);
  }
  
  startedWaiting = millis();
  while (HIGH != digitalRead(PN5180_BUSY)) {
    if (millis() - startedWaiting > commandTimeout) return false;
  }
  
  digitalWrite(PN5180_NSS, HIGH);
  delayMicroseconds(50);
  
  startedWaiting = millis();
  while (LOW != digitalRead(PN5180_BUSY)) {
    if (millis() - startedWaiting > commandTimeout) return false;
  }

  return true;
}

void PN5180::reset() {
  digitalWrite(PN5180_RST, LOW);
  delay(50);
  digitalWrite(PN5180_RST, HIGH);
  delay(100);

  uint32_t status = 0;
  unsigned long start = millis();
  while (0 == (IDLE_IRQ_STAT & status)) {
    if (millis() - start > 1000) break;
    readRegister(IRQ_STATUS, &status);
  }
  clearIRQStatus(0xffffffff);
}

uint32_t PN5180::getIRQStatus() {
  uint32_t irqStatus = 0;
  readRegister(IRQ_STATUS, &irqStatus);
  return irqStatus;
}

bool PN5180::clearIRQStatus(uint32_t irqMask) {
  return writeRegister(IRQ_CLEAR, irqMask);
}

PN5180TransceiveStat PN5180::getTransceiveState() {
  uint32_t rfStatus = 0;
  if (!readRegister(RF_STATUS, &rfStatus)) return PN5180_TS_Idle;
  return PN5180TransceiveStat((rfStatus >> 24) & 0x07);
}