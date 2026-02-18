// =====================================================================
//  BaseScreen.cpp - Base screen implementation
// =====================================================================

#include "BaseScreen.h"

BaseScreen::BaseScreen(ScreenManager& manager, TFT_Module& tftModule)
    : screenManager(manager), tft(tftModule)
{
}
