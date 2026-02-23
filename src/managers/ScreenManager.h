// =====================================================================
//  ScreenManager.h - Manages screen switching and touch events
// =====================================================================

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H
#include "../screens/SettingsScreen.h"
#include "../screens/WriteTagScreen.h"
#include "../screens/WebUploadScreen.h"

class TFT_Module;
class BaseScreen;
class SplashScreen;
class MP3Screen;
class KidScreen;
class CalibrationScreen;
class VS1053_Module;
class SD_Module;  // Add this forward declaration

class ScreenManager {
public:
    ScreenManager(TFT_Module& tft, VS1053_Module& audio, SD_Module& sd, RC522_Module& nfc);
    ~ScreenManager();

    void begin();
    void update();
    void handleTouch(int x, int y);
    
    // Check if current screen is calibration screen
    bool isOnCalibrationScreen() const;

    // Navigation helpers
    void showSplash();
    void showMP3();
    void showKids();
    void showCalibration();
    void handleSongEnd(); 
    void showSettings();
    void showWriteTag();
    void showWebUpload();
    
    // Access to screens (for inter-screen communication)
    KidScreen* getKidScreen() { return kidScreen; }
    MP3Screen* getMP3Screen() { return mp3screen; }
    
    // Get TFT reference for screens
    TFT_Module& getTFT() { return tft; }

private:
    void switchTo(BaseScreen* newScreen);
    SettingsScreen* settingsScreen;
    WriteTagScreen* writeTagScreen;
    WebUploadScreen* webUploadScreen;

    
    TFT_Module& tft;
    VS1053_Module& audioModule;
    SD_Module& sdModule; 
    RC522_Module& nfcModule;
    
    BaseScreen* currentScreen;
    SplashScreen* splashScreen;
    MP3Screen* mp3screen;
    KidScreen* kidScreen;
    CalibrationScreen* calibrationScreen;
};

#endif // SCREEN_MANAGER_H
