// =====================================================================
//  SplashScreen.cpp - Welcome screen implementation
// =====================================================================

#include "SplashScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>

SplashScreen::SplashScreen(ScreenManager& manager, TFT_Module& tftModule)
    : BaseScreen(manager, tftModule),
      kidsButton(60, 140, 160, 80, "Kids Mode"),
      mp3Button(260, 140, 160, 80, "MP3 Mode"),
      calibrateButton(160, 240, 160, 50, "Calibrate")
{
    // Customize button colors
    kidsButton.setColors(0x07E0, TFT_YELLOW, TFT_WHITE);   // Green bg, yellow border
    mp3Button.setColors(0x001F, TFT_CYAN, TFT_WHITE);    // Blue bg, cyan border
    calibrateButton.setColors(0x632C, TFT_WHITE, TFT_WHITE); // Gray bg
}

void SplashScreen::begin() {
    auto display = tft.getTFT();
    
    // Clear screen with gradient or solid color
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
    
    // Draw buttons
    kidsButton.draw(tft);
    mp3Button.draw(tft);
    calibrateButton.draw(tft);
}

void SplashScreen::update() {
    // No animations on splash screen
}

void SplashScreen::handleTouch(int x, int y) {
    if (kidsButton.hit(x, y)) {
        Serial.println("Kids Mode selected");
        screenManager.showKids();
        return;
    }
    
    if (mp3Button.hit(x, y)) {
        Serial.println("MP3 Mode selected");
        screenManager.showMP3();
        return;
    }
    
    if (calibrateButton.hit(x, y)) {
        Serial.println("Calibrate selected");
        screenManager.showCalibration();
        return;
    }
}
