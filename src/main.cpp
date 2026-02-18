#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <FT6236.h>
#include "pins.h"
#include "utils/RC522_Module.h"
#include "utils/VS1053_Module.h"
#include "utils/TFT_Module.h"
#include "managers/ScreenManager.h"
#include "managers/ScreenManager.h"
#include "screens/KidScreen.h"  // Add this

// Hardware modules
RC522_Module nfcModule(NFC_CS, NFC_RST);
VS1053_Module audioModule(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RST);
TFT_Module tftModule(TFT_CS, TFT_DC, TFT_RST, TFT_BL, SPI2_SCK, SPI2_MOSI, SPI2_MISO);
FT6236 touchScreen;

// Screen management
ScreenManager screenManager(tftModule);

// Touch interrupt
volatile bool touchDetected = false;

void IRAM_ATTR touchISR() {
  touchDetected = true;
}

// NFC state tracking
bool nfcTagPresent = false;
unsigned long lastNFCCheck = 0;
const unsigned long NFC_CHECK_INTERVAL = 500;  // Check every 500ms
// Forward declaration
void checkNFCStatus();

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
  
  pinMode(TOUCH_INT, INPUT);
  attachInterrupt(TOUCH_INT, touchISR, FALLING);
  Serial.println("Touch: ✓ Ready!");
  
  // Initialize Screen Manager (shows splash screen)
  Serial.println("\nInitializing Screen Manager...");
  screenManager.begin();
  
  Serial.println("\n=== System Ready ===\n");
}

void loop() {
  // Update current screen (animations, etc.)
  screenManager.update();
  
  // Handle touch events
  if (touchDetected) {
    touchDetected = false;
    TS_Point p = touchScreen.getPoint();
    
    if (p.x != 0 && p.y != 0) {  // Filter phantom touches
      Serial.printf("Touch: X=%d Y=%d\n", p.x, p.y);
      screenManager.handleTouch(p.x, p.y);
    }
  }
  
  // Check for NFC tags (in Kid Mode only)
  if (millis() - lastNFCCheck >= NFC_CHECK_INTERVAL) {
    lastNFCCheck = millis();
    checkNFCStatus();
  }
  
  delay(10);
}

void checkNFCStatus() {
  bool tagNowPresent = nfcModule.isCardPresent();
  
  // Tag just placed
  if (tagNowPresent && !nfcTagPresent) {
    Serial.println("NFC: Tag detected!");
    
    // Read album text from tag
    char albumText[40];
    if (nfcModule.readAlbumText(albumText, sizeof(albumText))) {
      Serial.printf("NFC: Album = '%s'\n", albumText);
      
      // Show album on kid screen
      screenManager.getKidScreen()->showAlbum(albumText);
    }
    
    nfcModule.haltCard();
  }
  
  // Tag just removed
  else if (!tagNowPresent && nfcTagPresent) {
    Serial.println("NFC: Tag removed!");
    
    // Clear album and return to splash
    screenManager.getKidScreen()->clearAlbum();
  }
  
  nfcTagPresent = tagNowPresent;
}