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
#include "utils/SD_Module.h"
#include "managers/MP3Player.h"
#include <WiFi.h>
#include <time.h>
#include "utils/Settings.h"

// Hardware modules
RC522_Module nfcModule(NFC_CS, NFC_RST);
VS1053_Module audioModule(VS1053_CS, VS1053_DCS, VS1053_DREQ, VS1053_RST);
TFT_Module tftModule(TFT_CS, TFT_DC, TFT_RST, TFT_BL, SPI2_SCK, SPI2_MOSI, SPI2_MISO);
SD_Module sdModule(SD_CS);
FT6236 touchScreen;
MP3Player mp3Player(sdModule, audioModule);

SdFat sd;  // ADD THIS - global SD object for web server

// Screen management
ScreenManager screenManager(tftModule, audioModule, sdModule, nfcModule); 

// Touch interrupt
volatile bool touchDetected = false;
// Time
bool timeValid = false;

// Task handle
TaskHandle_t mp3TaskHandle = NULL;

// WiFi credentials (hard-coded for now)
//const char* WIFI_SSID = "SSID";
//const char* WIFI_PASSWORD = "password";

// NTP server
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = -28800;  // PST = UTC-8 hours
const int DAYLIGHT_OFFSET_SEC = 3600;  // DST adjustment

bool getFormattedTime(char* buffer, size_t bufferSize) {
    if (!timeValid) return false;
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return false;
    
    strftime(buffer, bufferSize, "%a %b %d, %I:%M %p", &timeinfo);  // "Sun Feb 22, 12:04 PM"
    return true;
}

void setupWiFi() {
    Serial.println("\n=== Connecting to WiFi ===");
    
    // Get settings
    Settings& settings = Settings::getInstance();
    
    if (settings.wifi_ssid.length() == 0) {
        Serial.println("No WiFi credentials in settings, skipping WiFi");
        return;
    }
    
    Serial.printf("SSID: %s\n", settings.wifi_ssid.c_str());
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    delay(2000);  // Hardware settle time
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(settings.wifi_ssid.c_str(), settings.wifi_password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < settings.wifi_timeout) {
        delay(600);
        Serial.print(".");
        if (attempts % 5 == 0) {
            Serial.printf(" [%d] ", WiFi.status());
        }
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi Connected!");
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        
        // Get time from NTP
        Serial.println("Syncing time from NTP...");
        configTime(settings.ntp_offset, settings.ntp_daylight, "pool.ntp.org");
        
        int timeouts = 0;
        while (time(nullptr) < 100000 && timeouts < 40) {
            delay(500);
            Serial.print(".");
            timeouts++;
        }
        
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            char timeStr[64];
            strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo);
            Serial.printf("\n✓ Time: %s\n", timeStr);
            timeValid = true;
        }
        
    } else {
        Serial.println("\n✗ WiFi connection failed");
        Serial.printf("Final status: %d\n", WiFi.status());
    }
}


// Task function running on Core 0
void mp3StreamTask(void* parameter) {
    MP3Player* player = (MP3Player*)parameter;
    
    while (true) {
        player->update();
        vTaskDelay(1);  // Yield 1ms
    }
}

void IRAM_ATTR touchISR() {
  touchDetected = true;
}

void setup() {
  Serial.begin(115200);
  delay(4000);
  
  Serial.println("\n=== NFC MP3 Player Starting ===\n");


  
  // Hard reset RC522 first (before any SPI init)
  pinMode(NFC_RST, OUTPUT);
  digitalWrite(NFC_RST, LOW);
  delay(200);
  digitalWrite(NFC_RST, HIGH);
  delay(200);
  
  // Initialize SPI1 bus (RC522 + VS1053 + SD)
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
  audioModule.setVolume(55);
  
  // TEST: Verify audio output works
  Serial.println("Testing audio output...");
  audioModule.playTestTone(261);  //middle C
  delay(500);
  audioModule.stopPlayback();
  Serial.println("Audio test complete");
  
  delay(100);
  
  // Initialize SD Card
  Serial.println("\nInitializing SD Card...");
  if (!sdModule.begin()) {
    Serial.println("SD Card failed - MP3 playback won't work!");
  } else {
    Serial.println("SD Card ready!");
  }
  delay(100);

    // Load settings AFTER SD card init
    Settings& settings = Settings::getInstance();
    settings.load();

    Serial.printf("Free heap before WiFi: %d bytes\n", ESP.getFreeHeap());
    // Connect to WiFi
    setupWiFi();
    Serial.printf("Free heap after WiFi: %d bytes\n", ESP.getFreeHeap());

  // Create MP3 streaming task on Core 0 (AFTER SD init, BEFORE TFT)
  Serial.println("\nCreating MP3 streaming task on Core 0...");
  xTaskCreatePinnedToCore(
      mp3StreamTask,
      "MP3Stream",
      10000,
      &mp3Player,
      1,
      &mp3TaskHandle,
      0
  );
  Serial.println("MP3 task created");
  
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
  
  // Load saved calibration
  TouchCalibration::getInstance().loadFromPreferences();
  
  // Initialize Screen Manager
  Serial.println("\nInitializing Screen Manager...");
  screenManager.begin();


  
  Serial.println("\n=== System Ready ===\n");
}

void loop() {
  // DO NOT call mp3Player.update() - it runs on Core 0
  static bool wasPlaying = false;
  bool nowPlaying = mp3Player.isPlaying();

    // Only auto-advance if user didn't just manually advance
    static unsigned long lastManualAdvance = 0;
    if (wasPlaying && !nowPlaying && !mp3Player.isPlaying() && 
        millis() - lastManualAdvance > 1000) {  // 1 second debounce
        screenManager.handleSongEnd();
    }
    wasPlaying = nowPlaying;

  // Update screen animations
  screenManager.update();
  
  // Monitor NFC tags
  nfcModule.monitorForTags(screenManager);
  
  // Handle touch events
  if (touchDetected) {
    touchDetected = false;
    delay(50);  // Debounce
    
    TS_Point p = touchScreen.getPoint();
    if (p.x != 0 && p.y != 0) {
      screenManager.handleTouch(p.x, p.y);
      lastManualAdvance = millis();  // Mark manual advance time    
    }
  }
}