// =====================================================================
//  RC522_Module.cpp - RC522 NFC Reader Implementation
// =====================================================================

#include "RC522_Module.h"
#include "../managers/ScreenManager.h"  // Add this
#include "../screens/KidScreen.h"        // And this

// Constructor - initialize rfid object in initializer list
RC522_Module::RC522_Module(uint8_t cs, uint8_t rst)
  : _cs(cs), _rst(rst), rfid(cs, rst)  // Initialize MFRC522 in initializer list
{
}

void RC522_Module::begin() {
  Serial.println("RC522: Initializing...");
  
  // Initialize RC522 (SPI already started in main)
  rfid.PCD_Init();
  delay(50);
  
  // Read firmware version to verify communication
  byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
  Serial.printf("RC522: Firmware version 0x%02X\n", version);
  
  if (version == 0x00 || version == 0xFF) {
    Serial.println("RC522: ✗ Communication failure - check wiring");
  } else {
    Serial.println("RC522: ✓ Ready!");
  }
}

void RC522_Module::monitorForTags(ScreenManager& screenManager) {
    static bool inCardSession = false;
    static unsigned long noReadCount = 0;
    static unsigned long lastCheck = 0;
    static byte lastUID[10];
    static byte lastUIDSize = 0;
    const unsigned long CHECK_INTERVAL = 500;
    const unsigned long NO_READ_THRESHOLD = 4;
    
    if (millis() - lastCheck < CHECK_INTERVAL) return;
    lastCheck = millis();
    
    // Don't call isCardPresent - go straight to reading
    if (rfid.PICC_IsNewCardPresent()) {
        if (rfid.PICC_ReadCardSerial()) {
            noReadCount = 0;
            
            if (!inCardSession) {
                // New session
                Serial.println("NFC: Card detected");
                inCardSession = true;
                
                // Save UID
                lastUIDSize = rfid.uid.size;
                for (byte i = 0; i < lastUIDSize; i++) {
                    lastUID[i] = rfid.uid.uidByte[i];
                }
                
                char albumText[40];
                if (readAlbumText(albumText, sizeof(albumText))) {
                    Serial.printf("NFC: Album = '%s'\n", albumText);
                    screenManager.getKidScreen()->showAlbum(albumText);
                }
            }
            // Don't halt - let it keep reading
        }
    }
    else if (inCardSession) {
        noReadCount++;
        if (noReadCount >= NO_READ_THRESHOLD) {
            Serial.println("NFC: Card removed");
            inCardSession = false;
            noReadCount = 0;
            screenManager.getKidScreen()->clearAlbum();
            screenManager.showSplash();
        }
    }
}

bool RC522_Module::isCardPresent() {
  return rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial();
}

void RC522_Module::haltCard() {
  rfid.PICC_HaltA();
}

bool RC522_Module::waitForCard(uint32_t timeout_ms) {
  uint32_t start = millis();
  
  while (millis() - start < timeout_ms) {
    if (rfid.PICC_IsNewCardPresent()) {
      if (rfid.PICC_ReadCardSerial()) {
        return true;
      }
    }
    delay(50);
  }
  
  return false;
}

bool RC522_Module::readUserData(uint8_t *buffer, uint8_t numBytes) {
  uint8_t startPage = 4;
  uint8_t numPages = (numBytes + 3) / 4;  // 4 bytes per page
  
  for (uint8_t i = 0; i < numPages; i++) {
    byte readBuffer[18];
    byte size = sizeof(readBuffer);
    
    MFRC522::StatusCode status = rfid.MIFARE_Read(startPage + i, readBuffer, &size);
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
  // NDEF structure:
  // Byte 0: 0x03 (message start)
  // Byte 5: 0x54 ('T' for text record type)
  // Byte 9+: Text data starts here
  
  if (ndefData[0] != 0x03) return 0;   // Not NDEF
  if (ndefData[5] != 0x54) return 0;   // Not text record
  
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

bool RC522_Module::readAlbumText(char *textOut, uint8_t maxLen) {
  uint8_t userData[48];
  
  if (!readUserData(userData, 48)) {
    return false;
  }
  
  int textLen = extractText(userData, textOut, maxLen);
  return textLen > 0;
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
    for (byte j = 0; j < rfid.uid.size; j++) {
      Serial.printf("%02X ", rfid.uid.uidByte[j]);
    }
    Serial.println();
    
    // Read album text
    char cleanText[40];
    if (readAlbumText(cleanText, sizeof(cleanText) - 1)) {
      Serial.print("Album: \"");
      Serial.print(cleanText);
      Serial.println("\"");
    } else {
      // Fallback to raw data
      uint8_t userData[48];
      if (readUserData(userData, 48)) {
        Serial.print("Raw: ");
        for (int k = 0; k < 20; k++) {
          char c = userData[k];
          Serial.print((c >= 32 && c <= 126) ? c : '.');
        }
        Serial.println();
      } else {
        Serial.println("Failed to read user data");
      }
    }
    
    haltCard();
    delay(500);
  }
}