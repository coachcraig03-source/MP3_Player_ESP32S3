// =====================================================================
//  UIButton.cpp - Button widget implementation
// =====================================================================

#include "UIButton.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>

UIButton::UIButton()
    : x(0), y(0), w(0), h(0), label(""),
      bgColor(0x1F7C), borderColor(TFT_WHITE), textColor(TFT_WHITE)
{
}

UIButton::UIButton(int x, int y, int w, int h, const char* label)
    : x(x), y(y), w(w), h(h), label(label),
      bgColor(0x1F7C), borderColor(TFT_WHITE), textColor(TFT_WHITE)
{
}

void UIButton::draw(TFT_Module& tftModule) {
    auto tft = tftModule.getTFT();
    
    // Draw button background
    tft->fillRoundRect(x, y, w, h, 8, bgColor);
    
    // Draw border
    tft->drawRoundRect(x, y, w, h, 8, borderColor);
    
    // Draw label centered
    tft->setTextColor(textColor);
    tft->setTextDatum(middle_center);
    tft->setTextSize(2);
    tft->drawString(label, x + w/2, y + h/2);
}

bool UIButton::hit(int tx, int ty) const {
    return (tx >= x && tx <= x + w && ty >= y && ty <= y + h);
}

void UIButton::setLabel(const char* newLabel) {
    label = newLabel;
}

void UIButton::setPosition(int newX, int newY) {
    x = newX;
    y = newY;
}

void UIButton::setSize(int newW, int newH) {
    w = newW;
    h = newH;
}

void UIButton::setColors(uint32_t bg, uint32_t border, uint32_t text) {
    bgColor = bg;
    borderColor = border;
    textColor = text;
}
