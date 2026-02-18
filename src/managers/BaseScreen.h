// =====================================================================
//  BaseScreen.h - Base screen with access to TFT
// =====================================================================

#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

#include "Screen.h"

class TFT_Module;
class ScreenManager;

class BaseScreen : public Screen {
public:
    BaseScreen(ScreenManager& manager, TFT_Module& tftModule);
    virtual ~BaseScreen() {}

    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void handleTouch(int x, int y) = 0;

protected:
    ScreenManager& screenManager;
    TFT_Module& tft;
};

#endif // BASE_SCREEN_H
