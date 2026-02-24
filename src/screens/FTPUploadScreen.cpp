// =====================================================================
//  FTPUploadScreen.cpp - FTP Server in AP Mode
// =====================================================================

#include "FTPUploadScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>
#include <SdFat.h>
#include "pins.h"

FTPUploadScreen::FTPUploadScreen(ScreenManager& manager, TFT_Module& tftModule, SD_Module& sd)
    : BaseScreen(manager, tftModule),
      sdModule(sd),
      ftpServer(nullptr),
      doneButton(160, 260, 160, 50, "Done"),
      serverActive(false),
      lastUpdate(0)
{
    doneButton.setColors(TFT_RED, TFT_WHITE, TFT_WHITE);
}

void FTPUploadScreen::begin() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);
    
    // Title
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->setTextSize(3);
    display->drawString("FTP Upload", 240, 20);
    
    // Instructions - Step 1
    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->drawString("1. Connect to WiFi:", 240, 70);
    display->setTextSize(2);
    display->setTextColor(TFT_YELLOW);
    display->drawString("MP3Player", 240, 95);
    
    // Step 2
    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->drawString("2. Password:", 240, 125);
    display->setTextSize(2);
    display->setTextColor(TFT_YELLOW);
    display->drawString("12345678", 240, 150);
    
    // Step 3
    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->drawString("3. Open FileZilla (or any FTP client)", 240, 180);
    
    // FTP connection details
    display->setTextSize(1);
    display->setTextColor(TFT_WHITE);
    display->drawString("Host: 192.168.4.1", 240, 205);
    display->drawString("Username: esp32", 240, 220);
    display->drawString("Password: esp32", 240, 235);
    display->drawString("Port: 21", 240, 250);
    
    doneButton.draw(tft);
    
    // Start FTP server
    startFTPServer();
}

void FTPUploadScreen::startFTPServer() {
    Serial.println("Starting FTP server in AP mode...");
    
    // Stop MP3 task
    extern TaskHandle_t mp3TaskHandle;
    if (mp3TaskHandle) {
        vTaskDelete(mp3TaskHandle);
        mp3TaskHandle = nullptr;
    }
    
    delay(500);
    
    // Start AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MP3Player", "12345678");
    delay(1000);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.printf("AP Started! IP: %s\n", IP.toString().c_str());
    
    // Initialize SD (library expects Arduino SD)
    if (SD.begin(SD_CS)) {  // Use your SD_CS pin
        Serial.println("SD opened for FTP");

        // List root directory contents
        File root = SD.open("/");
        Serial.println("Contents of root:");
        File file = root.openNextFile();
        while (file) {
            Serial.print(file.isDirectory() ? "DIR: " : "FILE: ");
            Serial.println(file.name());
            file = root.openNextFile();
        }
        
        ftpServer = new FtpServer();
        ftpServer->begin("esp32", "esp32");
        
        serverActive = true;
        Serial.println("FTP server started");
    } else {
        Serial.println("SD init failed for FTP");
    }
}

void FTPUploadScreen::stopFTPServer() {
    if (ftpServer) {
        delete ftpServer;
        ftpServer = nullptr;
    }
    
    // Return to station mode
    WiFi.mode(WIFI_STA);
    
    serverActive = false;
    Serial.println("FTP server stopped");
}

void FTPUploadScreen::update() {
    if (serverActive && ftpServer) {
        ftpServer->handleFTP();
        
        // Update status display every 2 seconds
        if (millis() - lastUpdate > 2000) {
            lastUpdate = millis();
            updateStatus();
        }
    }
}

void FTPUploadScreen::updateStatus() {
    auto display = tft.getTFT();
    
    // Show active indicator
    static bool blink = false;
    blink = !blink;
    display->fillCircle(450, 30, 10, blink ? TFT_GREEN : TFT_DARKGREEN);
}

void FTPUploadScreen::handleTouch(int x, int y) {
    if (doneButton.hit(x, y)) {
        Serial.println("Exiting FTP mode");
        stopFTPServer();
        screenManager.showSettings();
        return;
    }
}