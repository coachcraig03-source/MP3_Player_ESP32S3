// =====================================================================
//  ScreenManager.cpp - Screen management implementation
// =====================================================================

#include "ScreenManager.h"
#include "../screens/SplashScreen.h"
#include "../screens/MP3Screen.h"
#include "../screens/KidScreen.h"
#include "../screens/CalibrationScreen.h"
#include "../utils/TouchCalibration.h"

ScreenManager::ScreenManager(TFT_Module& tftRef, VS1053_Module& audio, SD_Module& sd, RC522_Module& nfc)
    : tft(tftRef),
      audioModule(audio),
      sdModule(sd),
      nfcModule(nfc),  // ADD THIS LINE
      currentScreen(nullptr),
      splashScreen(nullptr),
      mp3screen(nullptr),
      kidScreen(nullptr),
      calibrationScreen(nullptr),
      settingsScreen(nullptr),
      writeTagScreen(nullptr),
      webUploadScreen(nullptr)  // ADD THIS
{
}

ScreenManager::~ScreenManager() {
    delete splashScreen;
    delete mp3screen;
    delete kidScreen;
    delete calibrationScreen;
    delete settingsScreen;
    delete writeTagScreen;  
    delete webUploadScreen;  
}

void ScreenManager::begin() {
    // Create all screens
    splashScreen = new SplashScreen(*this, tft);
    mp3screen = new MP3Screen(*this, tft, sdModule, audioModule);
    kidScreen = new KidScreen(*this, tft, audioModule, sdModule);
    settingsScreen = new SettingsScreen(*this, tft);  
    writeTagScreen = new WriteTagScreen(*this, tft, sdModule, nfcModule);   
    webUploadScreen = new WebUploadScreen(*this, tft, sdModule);      
    // Start with splash screen
    switchTo(splashScreen);
}

void ScreenManager::showWebUpload() {
    switchTo(webUploadScreen);
}

void ScreenManager::showWriteTag() {
    switchTo(writeTagScreen);
}

void ScreenManager::showSplash() {
    switchTo(splashScreen);
}

void ScreenManager::showMP3() {
    switchTo(mp3screen);
}

void ScreenManager::showKids() {
    switchTo(kidScreen);
}

void ScreenManager::showSettings() {
    switchTo(settingsScreen);
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
        
        // Transform coordinates UNLESS we're on calibration screen
        if (currentScreen != calibrationScreen && TouchCalibration::getInstance().isCalibrated()) {
            TouchCalibration::getInstance().transform(x, y, screenX, screenY);
        }
        
        currentScreen->handleTouch(screenX, screenY);
    }
}

bool ScreenManager::isOnCalibrationScreen() const {
    return (currentScreen == calibrationScreen);
}

void ScreenManager::handleSongEnd() {
    if (currentScreen == kidScreen) {
        kidScreen->nextTrack();
    } else if (currentScreen == mp3screen) {
        mp3screen->nextTrack();
    }
}
