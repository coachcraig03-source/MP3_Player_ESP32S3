#ifndef NFC_MODULE_H
#define NFC_MODULE_H

#include <Arduino.h>
#include <PN5180ISO14443.h>

class NFC_Module {
public:
  NFC_Module(uint8_t cs, uint8_t busy, uint8_t rst);

  void begin();
  void dumpRegisters();

  bool readUserData(uint8_t *buffer, uint8_t numBytes);
  
  // Extract clean text from NDEF formatted data
  int extractText(const uint8_t *ndefData, char *textOut, uint8_t maxLen);

  // Run N reads for testing
  void runPNTest(int count);

private:
  int8_t readCardSerialWithWupa(uint8_t *buffer);

  uint8_t _cs, _busy, _rst;
  PN5180ISO14443 *nfc;
};

#endif
