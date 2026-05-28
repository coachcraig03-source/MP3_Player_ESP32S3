// =====================================================================
//  PN532_Module.h - PN532 NFC Reader Module (I2C)
//  Replaces RC522_Module, identical external interface
// =====================================================================

#ifndef PN532_MODULE_H
#define PN532_MODULE_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "pins.h"

// Forward declaration
class ScreenManager;

class PN532_Module {
public:
  // Constructor - I2C mode, no CS or RST pins needed
  PN532_Module();
  
  // Initialize the PN532
  void begin();
  
  // Write album text to NFC tag
  bool writeAlbumTag(const char* albumName);
  
  // Read user data from NTAG215 (starting at page 4)
  bool readUserData(uint8_t *buffer, uint8_t numBytes);

  // Monitor NFC for tag placement/removal with debouncing
  void monitorForTags(ScreenManager& screenManager);
    
  // Extract clean text from NDEF formatted data
  int extractText(const uint8_t *ndefData, char *textOut, uint8_t maxLen);
  
  // Run test routine (N reads)
  void runTest(int count);
  
  // Check if a card is present
  bool isCardPresent();
  
  // Get album text from current card
  bool readAlbumText(char *textOut, uint8_t maxLen);
  
  // Halt current card
  void haltCard();

private:
  Adafruit_PN532 nfc;
  uint8_t _uid[7];
  uint8_t _uidLength;
  
  // Wait for card with timeout
  bool waitForCard(uint32_t timeout_ms = 5000);
};

#endif // PN532_MODULE_H