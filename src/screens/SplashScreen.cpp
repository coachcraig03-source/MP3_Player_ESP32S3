#include "SplashScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>

SplashScreen::SplashScreen(ScreenManager& manager, TFT_Module& tftModule)
    : BaseScreen(manager, tftModule),
      kidsButton(60, 140, 160, 80, "Kids Mode"),
      mp3Button(260, 140, 160, 80, "MP3 Mode"),
      settingsButton(420, 10, 50, 50, "")  // Gear icon, top-right
{
    // Customize button colors
    kidsButton.setColors(0x07E0, TFT_YELLOW, TFT_WHITE);   // Green bg, yellow border
    mp3Button.setColors(0x001F, TFT_CYAN, TFT_WHITE);      // Blue bg, cyan border
    settingsButton.setColors(0x632C, TFT_WHITE, TFT_WHITE); // Gray bg
}

void SplashScreen::begin() {
    auto display = tft.getTFT();
    
    // Clear screen
    display->fillScreen(TFT_BLACK);
    
    // Title
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->setTextSize(3);
    display->drawString("NFC MP3 Player", 240, 40);
    
    // Subtitle
    display->setTextSize(2);
    display->setTextColor(TFT_CYAN);
    display->drawString("Select Mode", 240, 90);

        
    // Time display (bottom of screen)
    extern bool timeValid;
    if (timeValid) {
        char timeStr[64];
        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            strftime(timeStr, sizeof(timeStr), "%a %b %d, %I:%M %p", &timeinfo);
            display->setTextSize(1);
            display->setTextColor(TFT_LIGHTGREY);
            display->setTextDatum(bottom_center);
            display->drawString(timeStr, 240, 310);  // Bottom center
        }
    }
    
    // Draw mode buttons
    kidsButton.draw(tft);
    mp3Button.draw(tft);
    
    // Draw settings button with gear icon
    settingsButton.draw(tft);
    drawGearIcon(445, 35, 15);  // Center of settings button
}

void SplashScreen::drawGearIcon(int centerX, int centerY, int radius) {
    auto display = tft.getTFT();
    
    // Draw simple gear icon
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(middle_center);
    display->setTextSize(3);
    display->drawString("*", centerX, centerY);  // Temporary - will improve
    
    // TODO: Draw proper gear with teeth
    // For now, asterisk works as gear placeholder
}

void SplashScreen::update() {
    static unsigned long lastUpdate = 0;
    
    if (millis() - lastUpdate > 60000) {  // Update every minute
        lastUpdate = millis();
        
        extern bool timeValid;
        if (timeValid) {
            auto display = tft.getTFT();
            char timeStr[64];
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                // Clear old time
                display->fillRect(140, 295, 200, 20, TFT_BLACK);
                
                // Draw new time
                strftime(timeStr, sizeof(timeStr), "%a %b %d, %I:%M %p", &timeinfo);
                display->setTextSize(1);
                display->setTextColor(TFT_LIGHTGREY);
                display->setTextDatum(bottom_center);
                display->drawString(timeStr, 240, 310);
            }
        }
    }
}

void SplashScreen::handleTouch(int x, int y) {
    if (kidsButton.hit(x, y)) {
        Serial.println("Kids Mode selected");
        screenManager.showKids();
    }
    else if (mp3Button.hit(x, y)) {
        Serial.println("MP3 Mode selected");
        screenManager.showMP3();
    }
    else if (settingsButton.hit(x, y)) {
        Serial.println("Settings selected");
        screenManager.showSettings();
    }
}