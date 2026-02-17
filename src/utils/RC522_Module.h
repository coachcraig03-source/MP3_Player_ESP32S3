// =====================================================================
//  RC522_Module.h - RC522 NFC Reader Module
//  Encapsulates MFRC522 library for NTAG215 reading
// =====================================================================

#ifndef RC522_MODULE_H
#define RC522_MODULE_H

#include <Arduino.h>
#include <MFRC522.h>

class RC522_Module {
public:
  // Constructor
  RC522_Module(uint8_t cs, uint8_t rst);
  
  // Initialize the RC522
  void begin();
  
  // Read user data from NTAG215 (starting at page 4)
  bool readUserData(uint8_t *buffer, uint8_t numBytes);
  
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
  uint8_t _cs, _rst;
  MFRC522 rfid;  // Static object, NOT pointer
  
  // Wait for card with timeout
  bool waitForCard(uint32_t timeout_ms = 5000);
};

#endif // RC522_MODULE_H