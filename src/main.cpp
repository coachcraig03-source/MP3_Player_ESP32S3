#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"

MFRC522 rfid(NFC_CS, NFC_RST);

// Extract text from NDEF data
int extractText(const uint8_t *ndefData, char *textOut, uint8_t maxLen) {
  if (ndefData[0] != 0x03) return 0;
  if (ndefData[5] != 0x54) return 0;
  
  int textStart = 9;
  int textLen = 0;
  
  for (int i = textStart; i < maxLen + textStart && ndefData[i] != 0x00 && ndefData[i] != 0xFE; i++) {
    if (ndefData[i] >= 32 && ndefData[i] <= 126) {
      textOut[textLen++] = ndefData[i];
    }
  }
  textOut[textLen] = '\0';
  return textLen;
}

// Read NTAG215 user data pages
bool readUserData(uint8_t *buffer, uint8_t numBytes) {
  uint8_t startPage = 4;
  uint8_t numPages = (numBytes + 3) / 4;
  
  for (uint8_t i = 0; i < numPages; i++) {
    byte readBuffer[18];
    byte size = sizeof(readBuffer);
    
    MFRC522::StatusCode status = rfid.MIFARE_Read(startPage + i, readBuffer, &size);
    if (status != MFRC522::STATUS_OK) {
      return false;
    }
    
    for (uint8_t j = 0; j < 4 && (i*4 + j) < numBytes; j++) {
      buffer[i*4 + j] = readBuffer[j];
    }
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(4000);
  
  Serial.println("\n=== RC522 Test ===");
  
  SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
  rfid.PCD_Init();
  
  Serial.println("RC522 initialized!");
  Serial.print("Firmware version: 0x");
  Serial.println(rfid.PCD_ReadRegister(rfid.VersionReg), HEX);
  
  Serial.println("\nPlace a tag near the reader...");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;
  
  // Print UID
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.printf("%02X ", rfid.uid.uidByte[i]);
  }
  Serial.println();
  
  // Read and display album text
  uint8_t userData[48];
  if (readUserData(userData, 48)) {
    char cleanText[40];
    int textLen = extractText(userData, cleanText, sizeof(cleanText) - 1);
    
    if (textLen > 0) {
      Serial.print("Album: \"");
      Serial.print(cleanText);
      Serial.println("\"");
    } else {
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
  
  rfid.PICC_HaltA();
  delay(1000);
}