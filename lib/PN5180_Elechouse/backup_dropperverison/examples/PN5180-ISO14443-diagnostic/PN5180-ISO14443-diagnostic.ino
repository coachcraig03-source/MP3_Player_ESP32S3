// PN5180 ISO14443 Diagnostic Sketch
// Based on PN5180-ISO14443.ino with full status dump each loop.

struct StatusSnapshot;

#include <Arduino.h>
#include <PN5180.h>
#include <PN5180ISO14443.h>

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560) || defined(ARDUINO_AVR_NANO)

#define PN5180_NSS  10
#define PN5180_BUSY 9
#define PN5180_RST  7
#define PN5180_SHUTDOWN_PIN -1 // not used on AVR example

#elif defined(ARDUINO_ARCH_ESP32)

#define PN5180_NSS  16   // swapped with BUSY in original example
#define PN5180_BUSY 5
#define PN5180_RST  17
#define PN5180_SHUTDOWN_PIN 21 // your board: ESP32 D21 connected to shutdown (active low)

#else
#error Please define your pinout here!
#endif

const unsigned long DIAG_INTERVAL_MS = 1000; // delay between loops

PN5180ISO14443 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

const char *transceiveStateName(uint8_t state) {
  switch (state) {
    case 0: return "Idle";
    case 1: return "WaitTransmit";
    case 2: return "Transmitting";
    case 3: return "WaitReceive";
    case 4: return "WaitForData";
    case 5: return "Receiving";
    case 6: return "LoopBack";
    default: return "Reserved";
  }
}

void printIRQBitsInline(uint32_t irqStatus) {
  Serial.print(F("["));
  if (irqStatus & (1UL << 0)) Serial.print(F("RX "));
  if (irqStatus & (1UL << 1)) Serial.print(F("TX "));
  if (irqStatus & (1UL << 2)) Serial.print(F("IDLE "));
  if (irqStatus & (1UL << 3)) Serial.print(F("MODE_DETECTED "));
  if (irqStatus & (1UL << 4)) Serial.print(F("CARD_ACTIVATED "));
  if (irqStatus & (1UL << 5)) Serial.print(F("STATE_CHANGE "));
  if (irqStatus & (1UL << 6)) Serial.print(F("RFOFF_DET "));
  if (irqStatus & (1UL << 7)) Serial.print(F("RFON_DET "));
  if (irqStatus & (1UL << 8)) Serial.print(F("TX_RFOFF "));
  if (irqStatus & (1UL << 9)) Serial.print(F("TX_RFON "));
  if (irqStatus & (1UL << 10)) Serial.print(F("RF_ACTIVE_ERROR "));
  if (irqStatus & (1UL << 11)) Serial.print(F("TIMER0 "));
  if (irqStatus & (1UL << 12)) Serial.print(F("TIMER1 "));
  if (irqStatus & (1UL << 13)) Serial.print(F("TIMER2 "));
  if (irqStatus & (1UL << 14)) Serial.print(F("RX_SOF_DET "));
  if (irqStatus & (1UL << 15)) Serial.print(F("RX_SC_DET "));
  if (irqStatus & (1UL << 16)) Serial.print(F("TEMPSENS_ERROR "));
  if (irqStatus & (1UL << 17)) Serial.print(F("GENERAL_ERROR "));
  if (irqStatus & (1UL << 18)) Serial.print(F("HV_ERROR "));
  if (irqStatus & (1UL << 19)) Serial.print(F("LPCD "));
  Serial.print(F("]"));
}

uint32_t readReg32(uint8_t reg) {
  uint32_t val = 0;
  if (!nfc.readRegister(reg, &val)) {
    Serial.print(F("readRegister failed for 0x"));
    Serial.println(reg, HEX);
  }
  return val;
}

uint8_t readEEpromByte(uint8_t addr) {
  uint8_t buf[1] = {0xFF};
  if (!nfc.readEEprom(addr, buf, 1)) {
    Serial.print(F("readEEprom failed for 0x"));
    Serial.println(addr, HEX);
  }
  return buf[0];
}

void printUid(const uint8_t *uid, int8_t uidLen) {
  Serial.print(F("UID="));
  for (int i = 0; i < uidLen; i++) {
    if (i) Serial.print(F(":"));
    if (uid[i] < 0x10) Serial.print(F("0"));
    Serial.print(uid[i], HEX);
  }
  Serial.println();
}

struct StatusSnapshot {
  uint32_t irq;
  uint32_t irqEnable;
  uint32_t sysStatus;
  uint32_t sysConfig;
  uint32_t rfStatus;
  uint32_t rxStatus;
  uint32_t agc;
  uint8_t dpc;
  int busy;
  int shutdown;
};

bool captureStatus(StatusSnapshot &s) {
  s.busy = digitalRead(PN5180_BUSY);
  s.shutdown = (PN5180_SHUTDOWN_PIN >= 0) ? digitalRead(PN5180_SHUTDOWN_PIN) : -1;
  if (s.busy == HIGH) {
    return false; // avoid long SPI timeouts while BUSY is stuck high
  }
  s.irq = nfc.getIRQStatus();
  s.irqEnable = readReg32(IRQ_ENABLE);
  s.sysStatus = readReg32(SYSTEM_STATUS);
  s.sysConfig = readReg32(SYSTEM_CONFIG);
  s.rfStatus = readReg32(RF_STATUS);
  s.rxStatus = readReg32(RX_STATUS);
  s.agc = readReg32(AGC_REF_CONFIG);
  s.dpc = readEEpromByte(DPC_XI);
  return true;
}

void printStatusLine(const char *label, const StatusSnapshot &s, bool includeExtras) {
  uint8_t transceiveState = (uint8_t)((s.rfStatus >> 24) & 0x07);
  uint16_t rxLen = (uint16_t)(s.rxStatus & 0x01FF);
  uint8_t rxCollision = (uint8_t)((s.rxStatus >> 18) & 0x01);
  uint8_t sysTrx = (uint8_t)(s.sysConfig & 0x07);

  Serial.print(label);
  Serial.print(F(": BUSY="));
  Serial.print(s.busy);
  if (s.shutdown >= 0) {
    Serial.print(F(" SHUTDOWN="));
    Serial.print(s.shutdown);
  }
  Serial.print(F(" IRQ=0x"));
  Serial.print(s.irq, HEX);
  Serial.print(F(" "));
  printIRQBitsInline(s.irq);
  Serial.print(F(" SYS=0x"));
  Serial.print(s.sysStatus, HEX);
  Serial.print(F(" CFG=0x"));
  Serial.print(s.sysConfig, HEX);
  Serial.print(F(" TRX=0x"));
  Serial.print(sysTrx, HEX);
  Serial.print(F(" RF=0x"));
  Serial.print(s.rfStatus, HEX);
  Serial.print(F(" TS="));
  Serial.print(transceiveState);
  Serial.print(F("("));
  Serial.print(transceiveStateName(transceiveState));
  Serial.print(F(")"));
  Serial.print(F(" RX=0x"));
  Serial.print(s.rxStatus, HEX);
  Serial.print(F(" len="));
  Serial.print(rxLen);
  Serial.print(F(" coll="));
  Serial.print(rxCollision);
  if (includeExtras) {
    Serial.print(F(" IRQ_EN=0x"));
    Serial.print(s.irqEnable, HEX);
    Serial.print(F(" AGC=0x"));
    Serial.print(s.agc, HEX);
    Serial.print(F(" DPC=0x"));
    Serial.print(s.dpc, HEX);
  }
  Serial.println();
}

void printBusySkipLine(const char *label, const StatusSnapshot &s) {
  Serial.print(label);
  Serial.print(F(": BUSY="));
  Serial.print(s.busy);
  if (s.shutdown >= 0) {
    Serial.print(F(" SHUTDOWN="));
    Serial.print(s.shutdown);
  }
  Serial.println(F(" BUSY_HIGH skip SPI reads"));
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("=================================="));
  Serial.println(F("Uploaded: " __DATE__ " " __TIME__));
  Serial.println(F("PN5180 ISO14443 Diagnostic Sketch"));

  nfc.begin();

  if (PN5180_SHUTDOWN_PIN >= 0) {
    pinMode(PN5180_SHUTDOWN_PIN, INPUT);
  }

  Serial.print(F("commandTimeout(ms)="));
  Serial.println(nfc.commandTimeout);

  Serial.println(F("----------------------------------"));
  Serial.println(F("PN5180 Hard-Reset..."));
  nfc.reset();

  Serial.println(F("----------------------------------"));
  Serial.println(F("Reading product version..."));
  uint8_t productVersion[2];
  nfc.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  Serial.print(F("Product version="));
  Serial.print(productVersion[1]);
  Serial.print(F("."));
  Serial.println(productVersion[0]);

  if (0xFF == productVersion[1]) {
    Serial.println(F("Initialization failed!?"));
    Serial.println(F("Press reset to restart..."));
    Serial.flush();
    exit(-1);
  }

  Serial.println(F("----------------------------------"));
  Serial.println(F("Reading firmware version..."));
  uint8_t firmwareVersion[2];
  nfc.readEEprom(FIRMWARE_VERSION, firmwareVersion, sizeof(firmwareVersion));
  Serial.print(F("Firmware version="));
  Serial.print(firmwareVersion[1]);
  Serial.print(F("."));
  Serial.println(firmwareVersion[0]);

  Serial.println(F("----------------------------------"));
  Serial.println(F("Reading EEPROM version..."));
  uint8_t eepromVersion[2];
  nfc.readEEprom(EEPROM_VERSION, eepromVersion, sizeof(eepromVersion));
  Serial.print(F("EEPROM version="));
  Serial.print(eepromVersion[1]);
  Serial.print(F("."));
  Serial.println(eepromVersion[0]);

  Serial.println(F("----------------------------------"));
  Serial.println(F("Enable RF field..."));
  nfc.setupRF();
}

uint32_t loopCnt = 0;

void loop() {
  static unsigned long busyHighStartMs = 0;

  Serial.println(F("----------------------------------"));
  Serial.print(F("Loop #"));
  Serial.println(loopCnt++);
  Serial.print(F("time="));
  Serial.print(millis());
  Serial.println(F(" ms"));

  StatusSnapshot pre;
  bool preValid = captureStatus(pre);
  if (preValid) {
    printStatusLine("PRE", pre, true);
  } else {
    printBusySkipLine("PRE", pre);
  }

  uint8_t uid[10] = {0};
  unsigned long readStartMs = millis();
  int8_t uidLen = nfc.readCardSerial(uid);
  unsigned long readMs = millis() - readStartMs;
  Serial.print(F("readCardSerial="));
  Serial.println(uidLen);
  Serial.print(F("readCardSerial_ms="));
  Serial.println(readMs);

  if (uidLen > 0) {
    printUid(uid, uidLen);
  } else if (uidLen == 0) {
    Serial.println(F("No card or invalid UID"));
  } else if (uidLen == -1) {
    Serial.println(F("readCardSerial: general error"));
  } else if (uidLen == -2) {
    Serial.println(F("readCardSerial: card in field but error"));
  } else {
    Serial.println(F("readCardSerial: unknown error"));
  }

  StatusSnapshot post;
  bool postValid = captureStatus(post);
  if (postValid) {
    printStatusLine("POST", post, false);
  } else {
    printBusySkipLine("POST", post);
  }

  if (post.busy == HIGH) {
    if (busyHighStartMs == 0) {
      busyHighStartMs = millis();
    }
  } else {
    busyHighStartMs = 0;
  }

  if (busyHighStartMs > 0) {
    Serial.print(F("BUSY_HIGH_MS="));
    Serial.println(millis() - busyHighStartMs);
  }

  delay(DIAG_INTERVAL_MS);
}
