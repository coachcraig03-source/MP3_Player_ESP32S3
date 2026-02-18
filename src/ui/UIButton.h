// =====================================================================
//  UIButton.h - Button widget for touch interface
// =====================================================================

#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <Arduino.h>

class TFT_Module;

class UIButton {
public:
    UIButton();
    UIButton(int x, int y, int w, int h, const char* label);

    void draw(TFT_Module& tft);
    bool hit(int tx, int ty) const;

    void setLabel(const char* label);
    void setPosition(int x, int y);
    void setSize(int w, int h);
    void setColors(uint32_t bg, uint32_t border, uint32_t text);

private:
    int x, y, w, h;
    const char* label;
    uint32_t bgColor;
    uint32_t borderColor;
    uint32_t textColor;
};

#endif // UI_BUTTON_H
