#include <Arduino.h>
#include "pins.h"
#include "utils/NFC_Module.h"

NFC_Module nfcModule(NFC_CS, NFC_BUSY, NFC_RST);

void printMenu() {
  Serial.println();
  Serial.println("Enter test you wish to run:");
  Serial.println("1: PN5180 test");
  Serial.println("2: TFT test");
  Serial.println("3: Touch test (no TFT)");
  Serial.println("4: Touch test (with TFT)");
  Serial.println("5: Touch calibration test");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  SPI.begin(SCK, MISO, MOSI);
  nfcModule.begin();

  printMenu();
}

void loop() {
  if (!Serial.available())
    return;

  char c = Serial.read();

  switch (c) {
    case '1':
      Serial.println("Running PN5180 test (20 reads)...");
      nfcModule.runPNTest(20);
      printMenu();
      break;

    case '2':
      Serial.println("TFT test not implemented yet");
      printMenu();
      break;

    case '3':
      Serial.println("Touch test (no TFT) not implemented yet");
      printMenu();
      break;

    case '4':
      Serial.println("Touch test (with TFT) not implemented yet");
      printMenu();
      break;

    case '5':
      Serial.println("Touch calibration test not implemented yet");
      printMenu();
      break;

    default:
      break;
  }
}
