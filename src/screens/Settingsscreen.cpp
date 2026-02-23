// =====================================================================
//  SettingsScreen.cpp - Settings and configuration implementation
// =====================================================================

#include "SettingsScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../utils/TouchCalibration.h"
#include <LovyanGFX.hpp>
#include <Preferences.h>

SettingsScreen::SettingsScreen(ScreenManager& manager, TFT_Module& tftModule)
    : BaseScreen(manager, tftModule),
      backButton(10, 10, 80, 40, "Back"),
      calibrateButton(300, 80, 140, 50, "Calibrate"),
      writeTagButton(300, 170, 140, 50, "Write Tag"), 
      saveButton(160, 260, 160, 50, "Save Settings"),
      webUploadButton(300, 260, 140, 50, "Web Upload")  // ADD THIS
{
    backButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    calibrateButton.setColors(0x07E0, TFT_WHITE, TFT_BLACK);  // Green
    writeTagButton.setColors(TFT_BLUE, TFT_WHITE, TFT_WHITE);  
    saveButton.setColors(TFT_BLUE, TFT_WHITE, TFT_WHITE);
    webUploadButton.setColors(TFT_BLUE, TFT_WHITE, TFT_WHITE);
}

void SettingsScreen::begin() {
    auto display = tft.getTFT();
    
    display->fillScreen(TFT_BLACK);
    
    // Title - centered
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->setTextSize(3);
    display->drawString("SETTINGS", 240, 15);
    
    // Draw buttons
    backButton.draw(tft);
    
    // Draw settings list
    drawSettingsList();
    
    // Save button (for future WiFi settings)
    // saveButton.draw(tft);  // Uncomment when we add WiFi settings
}

void SettingsScreen::drawSettingsList() {
    auto display = tft.getTFT();
    
    int yPos = 80;
    
    // Touch Calibration section
    display->fillRect(20, yPos, 440, 70, 0x2104);
    display->drawRect(20, yPos, 440, 70, TFT_CYAN);
    
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_left);
    display->setTextSize(2);
    display->drawString("Touch Screen", 30, yPos + 10);
    
    display->setTextSize(1);
    display->setTextColor(TFT_LIGHTGREY);
    display->drawString("Calibrate touch for accurate button presses", 30, yPos + 40);
    
    calibrateButton.draw(tft);
    
    // === NFC TAG WRITING SECTION ===
    yPos = 170;
    display->fillRect(20, yPos, 440, 70, 0x2104);
    display->drawRect(20, yPos, 440, 70, TFT_CYAN);
    
    display->setTextDatum(top_left);
    display->setTextSize(2);
    display->setTextColor(TFT_WHITE);
    display->drawString("NFC Tag Writing", 30, yPos + 10);
    
    display->setTextSize(1);
    display->setTextColor(TFT_LIGHTGREY);
    display->drawString("Write album names to blank NFC tags", 30, yPos + 40);
    
    writeTagButton.draw(tft);
    
    // === WEB UPLOAD SECTION ===
    yPos = 260;
    display->fillRect(20, yPos, 440, 70, 0x2104);
    display->drawRect(20, yPos, 440, 70, TFT_CYAN);

    display->setTextDatum(top_left);
    display->setTextSize(2);
    display->setTextColor(TFT_WHITE);
    display->drawString("WEB Upload", 30, yPos + 10);

    display->setTextSize(1);
    display->setTextColor(TFT_LIGHTGREY);
    display->drawString("Upload music files via WEB", 30, yPos + 40);


    webUploadButton.draw(tft);  // Change ftpButton to webUploadButton
    }

void SettingsScreen::startCalibration() {
    Serial.println("Starting touch calibration...");
    screenManager.showCalibration();
}

void SettingsScreen::update() {
    // No animation needed
}

void SettingsScreen::handleTouch(int x, int y) {
    if (backButton.hit(x, y)) {
        Serial.println("Returning to splash");
        screenManager.showSplash();
        return;
    }
    
    if (calibrateButton.hit(x, y)) {
        Serial.println("Calibrate button pressed");
        startCalibration();
        return;
    }

      
    if (writeTagButton.hit(x, y)) {  // ADD THIS
        Serial.println("Write Tag button pressed");
        screenManager.showWriteTag();
        return;
    }

    if (webUploadButton.hit(x, y)) {
        Serial.println("Web Upload button pressed");
        screenManager.showWebUpload();
    return;
}
    
    
    // Future: Handle WiFi settings, save button, etc.
}