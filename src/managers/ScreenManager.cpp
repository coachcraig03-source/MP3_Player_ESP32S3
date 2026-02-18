// =====================================================================
//  ScreenManager.cpp - Screen management implementation
// =====================================================================

#include "ScreenManager.h"
#include "../screens/SplashScreen.h"
#include "../screens/AdultScreen.h"
#include "../screens/KidScreen.h"
#include "../screens/CalibrationScreen.h"
#include "../utils/TouchCalibration.h"

ScreenManager::ScreenManager(TFT_Module& tftRef)
    : tft(tftRef),
      currentScreen(nullptr),
      splashScreen(nullptr),
      adultScreen(nullptr),
      kidScreen(nullptr),
      calibrationScreen(nullptr)
{
}

ScreenManager::~ScreenManager() {
    delete splashScreen;
    delete adultScreen;
    delete kidScreen;
    delete calibrationScreen;
}

void ScreenManager::begin() {
    // Create all screens
    splashScreen = new SplashScreen(*this, tft);
    adultScreen = new AdultScreen(*this, tft);
    kidScreen = new KidScreen(*this, tft);
    
    // Start with splash screen
    switchTo(splashScreen);
}

void ScreenManager::showSplash() {
    switchTo(splashScreen);
}

void ScreenManager::showAdult() {
    switchTo(adultScreen);
}

void ScreenManager::showKids() {
    switchTo(kidScreen);
}

void ScreenManager::showCalibration() {
    if (calibrationScreen == nullptr) {
        calibrationScreen = new CalibrationScreen(*this, tft);
    }
    switchTo(calibrationScreen);
}

void ScreenManager::switchTo(BaseScreen* newScreen) {
    currentScreen = newScreen;
    currentScreen->begin();
}

void ScreenManager::update() {
    if (currentScreen) {
        currentScreen->update();
    }
}

void ScreenManager::handleTouch(int x, int y) {
    if (currentScreen) {
        int screenX = x;
        int screenY = y;
        
        bool shouldTransform = (currentScreen != calibrationScreen && TouchCalibration::getInstance().isCalibrated());
        //Serial.printf("ScreenManager: shouldTransform=%d, isCalibrated=%d\n", shouldTransform, TouchCalibration::getInstance().isCalibrated());
        
        if (shouldTransform) {
            TouchCalibration::getInstance().transform(x, y, screenX, screenY);
            //Serial.printf("ScreenManager: Transform (%d,%d) -> (%d,%d)\n", x, y, screenX, screenY);
        }
        
        currentScreen->handleTouch(screenX, screenY);
    }
}

bool ScreenManager::isOnCalibrationScreen() const {
    return (currentScreen == calibrationScreen);
}
