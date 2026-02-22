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
      saveButton(160, 260, 160, 50, "Save Settings")
{
    backButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    calibrateButton.setColors(0x07E0, TFT_WHITE, TFT_BLACK);  // Green
    saveButton.setColors(TFT_BLUE, TFT_WHITE, TFT_WHITE);
}

void SettingsScreen::begin() {
    auto display = tft.getTFT();
    
    display->fillScreen(TFT_BLACK);
    
    // Title
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_left);
    display->setTextSize(2);
    display->drawString("SETTINGS", 100, 15);
    
    // Draw buttons
    backButton.draw(tft);
    
    // Draw settings list
    drawSettingsList();
    
    // Save button (for future WiFi settings)
    // saveButton.draw(tft);  // Uncomment when we add WiFi settings
}

void SettingsScreen::drawSettingsList() {
    auto display = tft.getTFT();
    
    int yPos = 70;
    
    // Touch Calibration section
    display->fillRect(20, yPos, 440, 60, TFT_DARKGREY);
    display->drawRect(20, yPos, 440, 60, TFT_WHITE);
    
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_left);
    display->setTextSize(2);
    display->drawString("Touch Screen", 30, yPos + 10);
    
    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->drawString("Calibrate touch for accurate button presses", 30, yPos + 35);
    
    calibrateButton.draw(tft);
    
    // WiFi section (placeholder for tomorrow)
    yPos = 150;
    display->fillRect(20, yPos, 440, 80, TFT_DARKGREY);
    display->drawRect(20, yPos, 440, 80, TFT_WHITE);
    
    display->setTextColor(TFT_WHITE);
    display->setTextSize(2);
    display->drawString("WiFi Upload", 30, yPos + 10);
    
    display->setTextSize(1);
    display->setTextColor(TFT_DARKGREY);  // Grayed out
    display->drawString("Coming soon: Upload music via WiFi", 30, yPos + 35);
    display->drawString("SSID: _______________", 30, yPos + 55);
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
    
    // Future: Handle WiFi settings, save button, etc.
}