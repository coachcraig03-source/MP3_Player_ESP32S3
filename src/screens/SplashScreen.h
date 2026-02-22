// =====================================================================
//  SplashScreen.h - Main menu / splash screen
// =====================================================================

#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

class ScreenManager;
class TFT_Module;

class SplashScreen : public BaseScreen {
public:
    SplashScreen(ScreenManager& manager, TFT_Module& tft);
    
    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    void drawGearIcon(int centerX, int centerY, int radius);
    
    UIButton kidsButton;
    UIButton mp3Button;
    UIButton settingsButton;  // Changed from calibrateButton
};

#endif // SPLASH_SCREEN_H