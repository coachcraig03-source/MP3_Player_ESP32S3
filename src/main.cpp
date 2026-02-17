#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "pins.h"

// Create RC522 instance using same pins as PN5180
MFRC522 rfid(NFC_CS, NFC_RST);  // CS=16, RST=18

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n=== RC522 Simple Test ===");
  
  // Initialize SPI
  SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);  // 12, 10, 11
  
  // Initialize RC522
  rfid.PCD_Init();
  
  Serial.println("RC522 initialized!");
  Serial.print("Firmware version: 0x");
  Serial.println(rfid.PCD_ReadRegister(rfid.VersionReg), HEX);
  
  Serial.println("\nPlace a tag near the reader...");
}

void loop() {
  // Look for new cards
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }
  
  // Read card UID
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }
  
  // Print UID
  Serial.print("UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println();
  
  rfid.PICC_HaltA();
  delay(1000);
}