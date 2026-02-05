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


  // NEW: run N reads
  void runPNTest(int count);

private:
  int8_t readCardSerialWithWupa(uint8_t *buffer);

  uint8_t _cs, _busy, _rst;
  PN5180ISO14443 *nfc;
};

#endif
