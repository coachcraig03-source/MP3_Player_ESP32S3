#include <PN5180.h>
#include <PN5180ISO14443.h>
#include "pins.h"
#include <Arduino.h>
#include "utils/NFC_Module.h"

#define PN5180_NSS   NFC_CS
#define PN5180_BUSY  NFC_BUSY
#define PN5180_RST   NFC_RST

// Our PN5180 class wrapper
NFC_Module nfcModule(PN5180_NSS, PN5180_BUSY, PN5180_RST);

static const unsigned long kBusyStuckMs = 5;
static const uint8_t kHardResetAfterErrors = 3;

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
  PN5180ISO14443* nfc = nfcModule.getNFC();

  uint8_t startPage = 4;
  uint8_t numReads = (numBytes + 15) / 16;

  for (uint8_t i = 0; i < numReads; i++) {
    uint8_t blockData[16];

    if (!nfc->mifareBlockRead(startPage + (i * 4), blockData)) {
      return false;
    }

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

  SPI.begin(SCK, MISO, MOSI);

  // FULL PN5180 startup + debug now happens inside the class
  nfcModule.begin();

  Serial.println("Wave NTAG215 tag to read UID and user data...");
  Serial.println();
}

void loop() {
  uint8_t uid[8];

  // Now using the class version
  int8_t uidLen = nfcModule.readCardSerialWithWupa(uid);

  if (uidLen == 0) {
    Serial.println("no card found");
    delay(200);
    return;
  }

  if (uidLen < 0) {
    Serial.println("Error reading card");
    Serial.println("Error - resetting PN5180...");
    nfcModule.getNFC()->reset();
    delay(10);
    nfcModule.getNFC()->setupRF();
    delay(200);
    return;
  }

  // Print UID
  Serial.print("UID: ");
  for (int i = 0; i < uidLen; i++) {
    Serial.printf("%02X ", uid[i]);
  }
  Serial.println();

  // Read 48 bytes of user data
  uint8_t userData[48];
  if (readUserData(userData, 48)) {
    Serial.print("Data: ");
    for (int i = 0; i < 48; i++) {
      if (userData[i] >= 32 && userData[i] <= 126) {
        Serial.print((char)userData[i]);
      } else {
        Serial.print('.');
      }
    }
    Serial.println();
  } else {
    Serial.println("Failed to read user data");
  }

  delay(1000);
}
