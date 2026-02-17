#include <Arduino.h>
#include <SPI.h>
#include "pins.h"
#include "utils/RC522_Module.h"
#include "utils/VS1053_Module.h"
#include "utils/TFT_Module.h"

// Create modules
RC522_Module nfcModule(NFC_CS, NFC_RST);
VS1053_Module audioModule(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RST);
TFT_Module tftModule(TFT_CS, TFT_DC, TFT_RST, TFT_BL, SPI2_SCK, SPI2_MOSI, SPI2_MISO);

void printMenu() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32-S3 Hardware Test Menu");
  Serial.println("========================================");
  Serial.println("1: RC522 NFC test (5 reads)");
  Serial.println("2: VS1053 audio test");
  Serial.println("========================================");
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(4000);
  
  Serial.println("\n=== ESP32-S3 Hardware Init ===\n");
  
  // Initialize SPI1 bus
  Serial.println("Initializing SPI1 bus...");
  SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
  delay(100);
  
  // Hold VS1053 in reset during RC522 init
  pinMode(VS1053_RST, OUTPUT);
  digitalWrite(VS1053_RST, LOW);
  delay(100);
  
  // Initialize RC522
  Serial.println("\nInitializing RC522 NFC...");
  nfcModule.begin();
  
  delay(100);
  
  // Release and initialize VS1053
  digitalWrite(VS1053_RST, HIGH);
  delay(100);
  
  Serial.println("\nInitializing VS1053 Audio...");
  audioModule.begin();
  delay(100);
  // Add after VS1053 module creation
  Serial.println("\nInitializing TFT..."); 
  tftModule.begin();
  delay(100);


  // Restore SPI1 after TFT corrupts it
  SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
  delay(100);

  Serial.println("\n=== Init Complete ===");
  printMenu();
}

void loop() {
  if (!Serial.available()) return;
  
  char c = Serial.read();
  
  switch (c) {
    case '1':
      Serial.println("\n>>> Running RC522 test (5 reads)...");
      nfcModule.runTest(5);
      printMenu();
      break;
    
    case '2':
      Serial.println("\n>>> Running VS1053 test...");
      if (audioModule.isAlive()) {
        Serial.println("✓ VS1053 is responding!");
        audioModule.getChipInfo();
      } else {
        Serial.println("✗ VS1053 not responding - check wiring");
      }
      printMenu();
      break;
    
    default:
      break;
  }
}