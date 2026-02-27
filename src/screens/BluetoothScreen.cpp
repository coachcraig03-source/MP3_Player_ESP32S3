// =====================================================================
//  BluetoothScreen.cpp - Bluetooth Audio Mode Implementation
// =====================================================================

#include "BluetoothScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../managers/MP3Player.h" 
#include <LovyanGFX.hpp>
#include "pins.h"


BluetoothScreen::BluetoothScreen(ScreenManager& manager, TFT_Module& tftModule)
    : BaseScreen(manager, tftModule),
      backButton(160, 260, 160, 50, "Back"),
      lastBlink(0),
      blinkState(false)
{
    backButton.setColors(TFT_RED, TFT_WHITE, TFT_WHITE);
}

void BluetoothScreen::begin() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);
    
    // Stop MP3 playback
    extern MP3Player mp3Player;
    mp3Player.stop();
    
    // Enable Bluetooth module
    pinMode(BT_ENABLE_PIN, OUTPUT);
    digitalWrite(BT_ENABLE_PIN, HIGH);
    Serial.println("Bluetooth mode enabled");
    
    // Title
    display->setTextColor(TFT_CYAN);
    display->setTextDatum(top_center);
    display->setTextSize(3);
    display->drawString("Bluetooth Mode", 240, 30);
    
    // Icon/indicator
    display->setTextSize(8);
    display->setTextColor(TFT_BLUE);
    display->drawString("B", 240, 100);  // Bluetooth symbol placeholder
    
    // Instructions
    display->setTextSize(2);
    display->setTextColor(TFT_WHITE);
    display->drawString("Pair your device", 240, 180);
    
    display->setTextSize(1);
    display->setTextColor(TFT_LIGHTGREY);
    display->drawString("Look for 'VHM-314' in Bluetooth settings", 240, 210);
    
    // Back button
    backButton.draw(tft);
    
    lastBlink = millis();
    blinkState = true;
}

void BluetoothScreen::update() {
    // Blink the Bluetooth icon
    if (millis() - lastBlink > 1000) {
        lastBlink = millis();
        blinkState = !blinkState;
        
        auto display = tft.getTFT();
        display->setTextSize(8);
        display->setTextColor(blinkState ? TFT_BLUE : TFT_DARKGREY);
        display->setTextDatum(top_center);
        display->drawString("B", 240, 100);
    }
}

void BluetoothScreen::handleTouch(int x, int y) {
    if (backButton.hit(x, y)) {
        Serial.println("Exiting Bluetooth mode");
        
        // Disable Bluetooth module
        digitalWrite(BT_ENABLE_PIN, LOW);
        
        // Return to splash screen
        screenManager.showSplash();
        return;
    }
}