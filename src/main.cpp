#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FT6236.h>
#include "pins.h"
#include "utils/RC522_Module.h"
#include "utils/VS1053_Module.h"
#include "utils/TFT_Module.h"
#include "utils/TouchCalibration.h"
#include "managers/ScreenManager.h"

// Hardware modules
RC522_Module nfcModule(NFC_CS, NFC_RST);
VS1053_Module audioModule(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RST);
TFT_Module tftModule(TFT_CS, TFT_DC, TFT_RST, TFT_BL, SPI2_SCK, SPI2_MOSI, SPI2_MISO);
FT6236 touchScreen;

// Screen management (pass audioModule reference)
ScreenManager screenManager(tftModule, audioModule);

// Touch interrupt
volatile bool touchDetected = false;

void IRAM_ATTR touchISR() {
  touchDetected = true;
}

void setup() {
  Serial.begin(115200);
  delay(4000);
  
  Serial.println("\n=== NFC MP3 Player Starting ===\n");
  
  // Initialize SPI1 bus (RC522 + VS1053)
  Serial.println("Initializing SPI1 bus...");
  SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
  delay(100);
  
  // Hold VS1053 in reset during RC522 init
  pinMode(VS1053_RST, OUTPUT);
  digitalWrite(VS1053_RST, LOW);
  delay(100);
  
  // Initialize RC522 NFC
  Serial.println("\nInitializing RC522 NFC...");
  nfcModule.begin();
  delay(100);
  
  // Release and initialize VS1053
  digitalWrite(VS1053_RST, HIGH);
  delay(100);
  
  Serial.println("\nInitializing VS1053 Audio...");
  audioModule.begin();
Serial.println("\nInitializing VS1053 Audio...");
audioModule.begin();
delay(100);

// TEST: Play tone immediately on boot
Serial.println("Testing VS1053 with tone...");
audioModule.playTestTone(1000);  // 1000Hz tone
delay(2000);
audioModule.stopPlayback();
Serial.println("Tone test complete");

// Initialize TFT
Serial.println("\nInitializing TFT Display...");


  delay(100);
  
  // Initialize TFT
  Serial.println("\nInitializing TFT Display...");
  tftModule.begin();
  delay(100);
  
  // Restore SPI1 after TFT init
  SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
  delay(100);
  
  // Initialize Touch
  Serial.println("\nInitializing Touch...");
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, LOW);
  delay(10);
  digitalWrite(TOUCH_RST, HIGH);
  delay(50);
  
  Wire.begin(I2C_SDA, I2C_SCL);
  touchScreen.begin();
  
  pinMode(TOUCH_INT, INPUT_PULLUP);
  attachInterrupt(TOUCH_INT, touchISR, FALLING);
  Serial.println("Touch: ✓ Ready!");
  
  // Set default calibration values
  TouchCalibration::getInstance().setCalibration(1.5, 1.3, -200, -50);
  
  // Load saved calibration (will override defaults if exists)
  TouchCalibration::getInstance().loadFromPreferences();
  
  // Initialize Screen Manager
  Serial.println("\nInitializing Screen Manager...");
  screenManager.begin();
  
  Serial.println("\n=== System Ready ===\n");
}

void loop() {
  // Update current screen (animations, etc.)
  screenManager.update();
  
  // Monitor NFC for tag placement/removal
  nfcModule.monitorForTags(screenManager);
  
  // Handle touch events
  if (touchDetected) {
    touchDetected = false;
    delay(50);  // Debounce
    
    TS_Point p = touchScreen.getPoint();
    
    if (p.x != 0 && p.y != 0) {
      // Pass RAW coordinates to ScreenManager (it handles transform)
      screenManager.handleTouch(p.x, p.y);
    }
  }
  
  delay(10);
}