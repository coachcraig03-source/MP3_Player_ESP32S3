// =====================================================================
//  ScreenManager.cpp - Screen management implementation
// =====================================================================

#include "ScreenManager.h"
#include "../screens/SplashScreen.h"
#include "../screens/AdultScreen.h"
#include "../screens/KidScreen.h"

ScreenManager::ScreenManager(TFT_Module& tftRef)
    : tft(tftRef),
      currentScreen(nullptr),
      splashScreen(nullptr),
      adultScreen(nullptr),
      kidScreen(nullptr)
{
}

ScreenManager::~ScreenManager() {
    delete splashScreen;
    delete adultScreen;
    delete kidScreen;
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
        currentScreen->handleTouch(x, y);
    }
}
