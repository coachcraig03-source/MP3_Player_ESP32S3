// =====================================================================
//  SplashScreen.h - Welcome screen with mode selection
// =====================================================================

#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

class SplashScreen : public BaseScreen {
public:
    SplashScreen(ScreenManager& manager, TFT_Module& tft);

    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    UIButton kidsButton;
    UIButton adultButton;
    UIButton calibrateButton;
};

#endif // SPLASH_SCREEN_H
