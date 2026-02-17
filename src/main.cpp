#include <Arduino.h>
#include <SPI.h>
#include "pins.h"
#include "utils/RC522_Module.h"
#include "utils/VS1053_Module.h"
#include "utils/TFT_Module.h"
#include "screens/TFT_TestScreen.h"
#include <Wire.h>
#include <FT6236.h>  // Changed from FT6X36.h

// Create modules
RC522_Module nfcModule(NFC_CS, NFC_RST);
VS1053_Module audioModule(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RST);
TFT_Module tftModule(TFT_CS, TFT_DC, TFT_RST, TFT_BL, SPI2_SCK, SPI2_MOSI, SPI2_MISO);
TFT_TestScreen tftTest(&tftModule);
FT6236 touchScreen;

volatile bool touchDetected = false;

void IRAM_ATTR touchISR() {
  touchDetected = true;
}


void printMenu() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32-S3 Hardware Test Menu");
  Serial.println("========================================");
  Serial.println("1: RC522 NFC test (5 reads)");
  Serial.println("2: TFT video test");
  Serial.println("3: VS1053 audio test");
  Serial.println("4: Touch screen test");
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

    // In setup(), after TFT init:
 
Serial.println("\nInitializing Touch...");

// Reset touch controller
pinMode(TOUCH_RST, OUTPUT);
digitalWrite(TOUCH_RST, LOW);
delay(10);
digitalWrite(TOUCH_RST, HIGH);
delay(50);

Wire.begin(I2C_SDA, I2C_SCL);  // Use the defines, not hardcoded 45, 46

// Scan I2C bus
Serial.println("Scanning I2C bus...");
for (byte addr = 1; addr < 127; addr++) {
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    Serial.printf("Found I2C device at 0x%02X\n", addr);
  }
}

touchScreen.begin();
pinMode(TOUCH_INT, INPUT);
attachInterrupt(TOUCH_INT, touchISR, FALLING);  // Touch triggers on falling edge
Serial.println("Touch: ✓ Ready!");

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
    Serial.println("\n>>> Running TFT display test...");
    tftTest.runTest();
    
    // Restore SPI1 after TFT test
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(100);
    
    printMenu();
    break;  // THIS WAS MISSING!
    
  case '3':
    Serial.println("\n>>> Running VS1053 test...");
    if (audioModule.isAlive()) {
      Serial.println("✓ VS1053 is responding!");
      audioModule.getChipInfo();
    } else {
      Serial.println("✗ VS1053 not responding - check wiring");
    }
    printMenu();
    break;  // THIS WAS MISSING!

  case '4':
  {
    Serial.println("\n>>> Touch test - tap screen 10 times...");
    int touchCount = 0;
    while (touchCount < 10) {
      if (touchDetected) {
        touchDetected = false;
        TS_Point p = touchScreen.getPoint();
        
        if (p.x != 0 && p.y != 0) {  // Filter phantom touches
          touchCount++;
          Serial.printf("Touch %d: X=%d Y=%d\n", touchCount, p.x, p.y);
        }
        delay(300);
      }
      delay(10);
    }
    Serial.println("Touch test complete!");
    printMenu();
    break;
  }
    
  default:
    break;
}
}