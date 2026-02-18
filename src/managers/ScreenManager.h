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

class ScreenManager {
public:
    ScreenManager(TFT_Module& tft);
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
    
    BaseScreen* currentScreen;
    SplashScreen* splashScreen;
    AdultScreen* adultScreen;
    KidScreen* kidScreen;
    CalibrationScreen* calibrationScreen;
};

#endif // SCREEN_MANAGER_H
