// =====================================================================
//  BluetoothScreen.h - Bluetooth Audio Mode
// =====================================================================

#ifndef BLUETOOTH_SCREEN_H
#define BLUETOOTH_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

class ScreenManager;
class TFT_Module;

class BluetoothScreen : public BaseScreen {
public:
    BluetoothScreen(ScreenManager& manager, TFT_Module& tft);
    
    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    UIButton backButton;
    unsigned long lastBlink;
    bool blinkState;
};

#endif // BLUETOOTH_SCREEN_H