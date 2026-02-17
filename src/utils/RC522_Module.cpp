// =====================================================================
//  RC522_Module.cpp - RC522 NFC Reader Implementation
// =====================================================================

#include "RC522_Module.h"

RC522_Module::RC522_Module(uint8_t cs, uint8_t rst)
  : _cs(cs), _rst(rst), rfid(nullptr)
{
}

void RC522_Module::begin() {
  Serial.println("RC522: Initializing...");
  
  // Create MFRC522 object
  rfid = new MFRC522(_cs, _rst);
  
  // Initialize RC522
  rfid->PCD_Init();
  
  // Read firmware version
  byte version = rfid->PCD_ReadRegister(rfid->VersionReg);
  Serial.printf("RC522: Firmware version 0x%02X\n", version);
  
  if (version == 0x00 || version == 0xFF) {
    Serial.println("RC522: ✗ Communication failure - check wiring");
  } else {
    Serial.println("RC522: ✓ Ready!");
  }
}

bool RC522_Module::readUserData(uint8_t *buffer, uint8_t numBytes) {
  uint8_t startPage = 4;
  uint8_t numPages = (numBytes + 3) / 4;  // 4 bytes per page
  
  for (uint8_t i = 0; i < numPages; i++) {
    byte readBuffer[18];
    byte size = sizeof(readBuffer);
    
    MFRC522::StatusCode status = rfid->MIFARE_Read(startPage + i, readBuffer, &size);
    if (status != MFRC522::STATUS_OK) {
      return false;
    }
    
    // Copy 4 bytes from this page
    for (uint8_t j = 0; j < 4 && (i*4 + j) < numBytes; j++) {
      buffer[i*4 + j] = readBuffer[j];
    }
  }
  
  return true;
}

int RC522_Module::extractText(const uint8_t *ndefData, char *textOut, uint8_t maxLen) {
  // NDEF structure with record header:
  // Byte 0: 0x03 (message start)
  // Byte 5: 0x54 ('T' for text record type)
  // Byte 9+: Text data starts here
  
  if (ndefData[0] != 0x03) {
    return 0;  // Not NDEF
  }
  
  // Check if byte 5 is 'T' (text record type)
  if (ndefData[5] != 0x54) {
    return 0;  // Not a text record
  }
  
  // Text starts at byte 9 (after "en")
  int textStart = 9;
  int textLen = 0;
  
  for (int i = textStart; i < maxLen + textStart && ndefData[i] != 0x00 && ndefData[i] != 0xFE; i++) {
    if (ndefData[i] >= 32 && ndefData[i] <= 126) {  // Printable ASCII
      textOut[textLen++] = ndefData[i];
    }
  }
  
  textOut[textLen] = '\0';
  return textLen;
}

bool RC522_Module::waitForCard(uint32_t timeout_ms) {
  uint32_t start = millis();
  
  while (millis() - start < timeout_ms) {
    if (rfid->PICC_IsNewCardPresent()) {
      if (rfid->PICC_ReadCardSerial()) {
        return true;
      }
    }
    delay(50);
  }
  
  return false;
}

void RC522_Module::runTest(int count) {
  for (int i = 0; i < count; i++) {
    Serial.printf("\n--- RC522 Read %d/%d ---\n", i + 1, count);
    Serial.println("Waiting for card...");
    
    if (!waitForCard(5000)) {
      Serial.println("No card found (timeout)");
      continue;
    }
    
    // Print UID
    Serial.print("UID: ");
    for (byte j = 0; j < rfid->uid.size; j++) {
      Serial.printf("%02X ", rfid->uid.uidByte[j]);
    }
    Serial.println();
    
    // Read user data
    uint8_t userData[48];
    if (readUserData(userData, 48)) {
      // Try to extract clean text
      char cleanText[40];
      int textLen = extractText(userData, cleanText, sizeof(cleanText) - 1);
      
      if (textLen > 0) {
        Serial.print("Album: \"");
        Serial.print(cleanText);
        Serial.println("\"");
      } else {
        // Fallback to raw data
        Serial.print("Raw: ");
        for (int k = 0; k < 20; k++) {
          char c = userData[k];
          Serial.print((c >= 32 && c <= 126) ? c : '.');
        }
        Serial.println();
      }
    } else {
      Serial.println("Failed to read user data");
    }
    
    // Halt card
    rfid->PICC_HaltA();
    delay(500);
  }
}