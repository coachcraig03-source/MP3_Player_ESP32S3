// =====================================================================
//  UISlider.h - Vertical Slider Widget
// =====================================================================

#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include <Arduino.h>

class TFT_Module;

class UISlider {
public:
    UISlider(int x, int y, int width, int height, int minVal = 0, int maxVal = 100);
    
    void draw(TFT_Module& tft);
    bool handleTouch(int touchX, int touchY);
    
    void setValue(int val);
    int getValue() const { return value; }
    
    void setColors(uint32_t track, uint32_t thumb, uint32_t border);

private:
    int x, y, width, height;
    int minValue, maxValue;
    int value;
    
    // Colors
    uint32_t trackColor;
    uint32_t thumbColor;
    uint32_t borderColor;
    
    int valueToY(int val);
    int yToValue(int touchY);
};

#endif // UI_SLIDER_H
