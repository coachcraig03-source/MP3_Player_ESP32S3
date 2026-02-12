#include <Arduino.h>
#include <SPI.h>
#include "pins.h"
#include "utils/NFC_Module.h"
#include "utils/VS1053_Module.h"

// Create NFC module
NFC_Module nfcModule(NFC_CS, NFC_BUSY, NFC_RST);

// Create VS1053 module
VS1053_Module audioModule(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RST);

void printMenu() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32-S3 Hardware Test Menu");
  Serial.println("========================================");
  Serial.println("1: PN5180 NFC test (20 reads)");
  Serial.println("2: TFT display test (not implemented)");
  Serial.println("3: VS1053 audio test");
  Serial.println("4: SD card test (not implemented)");
  Serial.println("5: Touch test (not implemented)");
  Serial.println("========================================");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== ESP32-S3 MP3 Player Hardware Init ===\n");
  
  // Initialize SPI1 bus (PN5180 + VS1053)
  Serial.println("Initializing SPI1 bus...");
  SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
  delay(100);
  
  // Hold VS1053 in reset during PN5180 init
  pinMode(VS1053_RST, OUTPUT);
  digitalWrite(VS1053_RST, LOW);  // Keep VS1053 off
  delay(100);

  // Initialize NFC module
  Serial.println("\nInitializing PN5180 NFC...");
  nfcModule.begin();
  delay(100);

  // Now release VS1053 and initialize it
  digitalWrite(VS1053_RST, HIGH);
  delay(100);
  // Initialize VS1053 module
  Serial.println("\nInitializing VS1053 Audio...");
  audioModule.begin();

// Manual CS test
Serial.println("\nManual CS test - watch GPIO 9:");
pinMode(9, OUTPUT);
for (int i = 0; i < 6; i++) {
  digitalWrite(9, LOW);
  Serial.println("CS LOW");
  delay(500);
  digitalWrite(9, HIGH);
  Serial.println("CS HIGH");
  delay(500);
}
  
  Serial.println("\n=== Hardware Initialization Complete ===");
  
  printMenu();
}

void loop() {
  if (!Serial.available())
    return;
  
  char c = Serial.read();
  
  switch (c) {
    case '1':
      Serial.println("\n>>> Running PN5180 test (20 reads)...");
      nfcModule.runPNTest(20);
      printMenu();
      break;
    
    case '2':
      Serial.println("\n>>> TFT test not implemented yet");
      printMenu();
      break;
    
    case '3':
      Serial.println("\n>>> Running VS1053 test...");
      Serial.println("Checking if VS1053 is alive...");
      if (audioModule.isAlive()) {
        Serial.println("✓ VS1053 is responding!");
        audioModule.getChipInfo();
      } else {
        Serial.println("✗ VS1053 not responding - check wiring");
      }
      printMenu();
      break;
    
    case '4':
      Serial.println("\n>>> SD card test not implemented yet");
      printMenu();
      break;
    
    case '5':
      Serial.println("\n>>> Touch test not implemented yet");
      printMenu();
      break;
    
    default:
      break;
  }
}