// =====================================================================
//  ScreenManager.h - Manages screen switching and touch events
// =====================================================================

#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

class TFT_Module;
class BaseScreen;
class SplashScreen;
class AdultScreen;
class KidScreen;
class CalibrationScreen;
class VS1053_Module;
class SD_Module;  // Add this forward declaration

class ScreenManager {
public:
    ScreenManager(TFT_Module& tft, VS1053_Module& audio, SD_Module& sd);
    ~ScreenManager();

    void begin();
    void update();
    void handleTouch(int x, int y);
    
    // Check if current screen is calibration screen
    bool isOnCalibrationScreen() const;

    // Navigation helpers
    void showSplash();
    void showAdult();
    void showKids();
    void showCalibration();
    
    // Access to screens (for inter-screen communication)
    KidScreen* getKidScreen() { return kidScreen; }
    AdultScreen* getAdultScreen() { return adultScreen; }
    
    // Get TFT reference for screens
    TFT_Module& getTFT() { return tft; }

private:
    void switchTo(BaseScreen* newScreen);
    
    TFT_Module& tft;
    VS1053_Module& audioModule;
    SD_Module& sdModule;  // Add this
    
    BaseScreen* currentScreen;
    SplashScreen* splashScreen;
    AdultScreen* adultScreen;
    KidScreen* kidScreen;
    CalibrationScreen* calibrationScreen;
};

#endif // SCREEN_MANAGER_H
