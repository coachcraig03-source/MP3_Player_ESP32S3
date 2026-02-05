#include "NFC_Module.h"

NFC_Module::NFC_Module(uint8_t cs, uint8_t busy, uint8_t rst)
  : _cs(cs), _busy(busy), _rst(rst), nfc(nullptr)
{
}

void NFC_Module::begin() {
  nfc = new PN5180ISO14443(_cs, _busy, _rst);

  Serial.println("PN5180 begin()...");
  nfc->begin();

  Serial.println("PN5180 Hard-Reset...");
  nfc->reset();

  Serial.println("Reading product version...");
  uint8_t productVersion[2];
  nfc->readEEprom(PRODUCT_VERSION, productVersion, sizeof(productVersion));
  Serial.printf("Product version=%d.%d\n", productVersion[1], productVersion[0]);

  dumpRegisters();

  Serial.println("Reading firmware version...");
  uint8_t firmwareVersion[2];
  nfc->readEEprom(FIRMWARE_VERSION, firmwareVersion, sizeof(firmwareVersion));
  Serial.printf("Firmware version=%d.%d\n", firmwareVersion[1], firmwareVersion[0]);

  Serial.println("Enable RF field via setupRF()...");
  nfc->setupRF();
  delay(500);
  dumpRegisters();

  Serial.println("Trying setRF_on()...");
  nfc->setRF_on();
  delay(500);
  dumpRegisters();

  Serial.println("Trying direct RF enable via register writes...");
  nfc->writeRegister(0x00, 0x00000003);
  delay(100);
  nfc->writeRegister(0x18, 0x00000001);
  delay(100);

  dumpRegisters();

  uint32_t rfCheck;
  nfc->readRegister(RF_STATUS, &rfCheck);
  Serial.printf("\nFinal RF_STATUS = 0x%08X ", rfCheck);
  if (rfCheck & 0x01) {
    Serial.println("✓ RF field is ON!");
  } else {
    Serial.println("✗ RF field still OFF");
  }
  Serial.println();
}

int8_t NFC_Module::readCardSerialWithWupa(uint8_t *buffer) {
  uint8_t response[10] = {0};

  int8_t uidLength = nfc->activateTypeA(response, 0);

  uint32_t rfStatus = 0;
  nfc->readRegister(RF_STATUS, &rfStatus);
  Serial.printf("RF_STATUS (loop) = 0x%08X\n", rfStatus);

  if (uidLength <= 0)
    uidLength = nfc->activateTypeA(response, 1);

  if (uidLength <= 0)
    return uidLength;

  if (uidLength < 4)
    return 0;

  for (int i = 0; i < uidLength; i++)
    buffer[i] = response[i + 3];

  return uidLength;
}


void NFC_Module::dumpRegisters() {
  uint32_t val;

  Serial.println("---- PN5180 Registers ----");

  nfc->readRegister(IRQ_STATUS, &val);
  Serial.printf("IRQ_STATUS    = 0x%08X\n", val);

  nfc->readRegister(RX_STATUS, &val);
  Serial.printf("RX_STATUS     = 0x%08X\n", val);

  nfc->readRegister(RF_STATUS, &val);
  Serial.printf("RF_STATUS     = 0x%08X\n", val);

  nfc->readRegister(SYSTEM_STATUS, &val);
  Serial.printf("SYSTEM_STATUS = 0x%08X\n", val);

  nfc->readRegister(MEMP_STATS, &val);
  Serial.printf("MEMP_STATS    = 0x%08X\n", val);

  Serial.println("---------------------------");
}
