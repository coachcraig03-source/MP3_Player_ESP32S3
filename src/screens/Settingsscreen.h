// =====================================================================
//  SettingsScreen.h - Settings and configuration screen
// =====================================================================

#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

class ScreenManager;
class TFT_Module;

class SettingsScreen : public BaseScreen {
public:
    SettingsScreen(ScreenManager& manager, TFT_Module& tft);
    
    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    void drawSettingsList();
    void startCalibration();
    
    UIButton backButton;
    UIButton calibrateButton;
    UIButton writeTagButton; 
    UIButton saveButton;
};

#endif // SETTINGS_SCREEN_H