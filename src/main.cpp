#include <PN5180.h>
#include <PN5180ISO14443.h>
#include "pins.h"
#include <Arduino.h>

#define PN5180_NSS   NFC_CS
#define PN5180_BUSY  NFC_BUSY
#define PN5180_RST   NFC_RST
#define PN5180_CE    -1   // You are not using CE

PN5180ISO14443 nfc(PN5180_NSS, PN5180_BUSY, PN5180_RST);

static const unsigned long kBusyStuckMs = 5;
static const uint8_t kHardResetAfterErrors = 3;

void dumpRegisters() {
    uint32_t val;

    Serial.println("---- PN5180 Registers ----");

    nfc.readRegister(IRQ_STATUS, &val);
    Serial.printf("IRQ_STATUS    = 0x%08X\n", val);

    nfc.readRegister(RX_STATUS, &val);
    Serial.printf("RX_STATUS     = 0x%08X\n", val);

    nfc.readRegister(RF_STATUS, &val);
    Serial.printf("RF_STATUS     = 0x%08X\n", val);

    nfc.readRegister(SYSTEM_STATUS, &val);
    Serial.printf("SYSTEM_STATUS = 0x%08X\n", val);

    nfc.readRegister(MEMP_STATS, &val);
    Serial.printf("MEMP_STATS    = 0x%08X\n", val);

    Serial.println("---------------------------");
}

static bool isBusyStuck(unsigned long waitMs) {
  unsigned long started = millis();
  while (millis() - started < waitMs) {
    if (digitalRead(PN5180_BUSY) == LOW) {
      return false;
    }
    delay(1);
  }
  return digitalRead(PN5180_BUSY) == HIGH;
}

// Read user data from NTAG215 (24 bytes from pages 4-9)
bool readUserData(uint8_t *buffer, uint8_t numBytes) {
  // NTAG215 pages are 4 bytes each
  // mifareBlockRead reads 16 bytes (4 pages) at a time
  uint8_t startPage = 4;
  uint8_t numReads = (numBytes + 15) / 16;  // Round up to 16-byte blocks
  
  for (uint8_t i = 0; i < numReads; i++) {
    uint8_t blockData[16];
    
    // Read 16 bytes starting from page (startPage + i*4)
    if (!nfc.mifareBlockRead(startPage + (i * 4), blockData)) {
      return false;
    }
    
    // Copy up to numBytes
    for (uint8_t j = 0; j < 16 && (i*16 + j) < numBytes; j++) {
      buffer[i*16 + j] = blockData[j];
    }
  }
  
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("PN5180 ISO14443 Demo with User Data Read");

  nfc.begin();

  Serial.println("PN5180 Hard-Reset...");
  nfc.reset();

  Serial.println("Reading product version...");
  uint8_t productVersion[2];
  nfc.readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  Serial.printf("Product version=%d.%d\n", productVersion[1], productVersion[0]);
  dumpRegisters();

  Serial.println("Reading firmware version...");
  uint8_t firmwareVersion[2];
  nfc.readEEprom(FIRMWARE_VERSION, firmwareVersion, sizeof(firmwareVersion));
  Serial.printf("Firmware version=%d.%d\n", firmwareVersion[1], firmwareVersion[0]);

  Serial.println("Enable RF field via setupRF()...");
  nfc.setupRF();
  delay(500);
  dumpRegisters();
  
  Serial.println("Trying setRF_on()...");
  nfc.setRF_on();
  delay(500);
  dumpRegisters();
  
  Serial.println("Trying direct RF enable via register writes...");
  // Write to SYSTEM_CONFIG register to enable RF
  nfc.writeRegister(0x00, 0x00000003);  // Enable TX_ENABLE and RX_ENABLE
  delay(100);
  
  // Also try the RF_CONTROL register
  nfc.writeRegister(0x18, 0x00000001);  // RF field ON
  delay(100);
  
  dumpRegisters();
  
  // Final check
  uint32_t rfCheck;
  nfc.readRegister(RF_STATUS, &rfCheck);
  Serial.printf("\nFinal RF_STATUS = 0x%08X ", rfCheck);
  if (rfCheck & 0x01) {
    Serial.println("✓ RF field is ON!");
  } else {
    Serial.println("✗ RF field still OFF");
    Serial.println("   Possible issues:");
    Serial.println("   - Antenna not connected to PN5180");
    Serial.println("   - PN5180 module defective");
    Serial.println("   - Wrong register addresses for this firmware");
  }
  Serial.println();
  Serial.println("Wave NTAG215 tag to read UID and 24 bytes of user data...");
  Serial.println();
}

int8_t readCardSerialWithWupa(uint8_t *buffer) {
  uint8_t response[10] = {0};
  int8_t uidLength = nfc.activateTypeA(response, 0);
  uint32_t rfStatus = 0;
  nfc.readRegister(RF_STATUS, &rfStatus);
  Serial.printf("RF_STATUS (loop) = 0x%08X\n", rfStatus);

  if (uidLength <= 0) uidLength = nfc.activateTypeA(response, 1);
  if (uidLength <= 0) return uidLength;
  if (uidLength < 4) return 0;

  for (int i = 0; i < uidLength; i++) buffer[i] = response[i + 3];
  return uidLength;
}

void loop() {
  uint8_t uid[8];
  int8_t uidLen = readCardSerialWithWupa(uid);

  if (uidLen == 0) {
    Serial.println("no card found");
    delay(200);
    return;
  }

  if (uidLen < 0) {
    Serial.println("Error reading card");
    Serial.println("Error - resetting PN5180...");
    nfc.reset();
    delay(10);
    nfc.setupRF();
    delay(200);
    return;
  }

  // Print UID
  Serial.print("UID: ");
  for (int i = 0; i < uidLen; i++) {
    Serial.printf("%02X ", uid[i]);
  }
  Serial.println();

  // Read 24 characters of user data
  uint8_t userData[48];
  if (readUserData(userData, 48)) {
    Serial.print("Data: ");
    for (int i = 0; i < 48; i++) {
      if (userData[i] >= 32 && userData[i] <= 126) {
        Serial.print((char)userData[i]);  // Printable ASCII
      } else {
        Serial.print('.');  // Non-printable
      }
    }
    Serial.println();
  } else {
    Serial.println("Failed to read user data");
  }

  delay(1000);
}