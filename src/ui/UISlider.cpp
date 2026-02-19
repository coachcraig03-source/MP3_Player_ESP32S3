// =====================================================================
//  UISlider.cpp - Vertical Slider Implementation
// =====================================================================

#include "UISlider.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>

UISlider::UISlider(int x, int y, int width, int height, int minVal, int maxVal)
    : x(x), y(y), width(width), height(height),
      minValue(minVal), maxValue(maxVal), value(maxVal),
      trackColor(0x4208),  // Dark gray
      thumbColor(TFT_WHITE),
      borderColor(TFT_DARKGREY)
{
}

void UISlider::setColors(uint32_t track, uint32_t thumb, uint32_t border) {
    trackColor = track;
    thumbColor = thumb;
    borderColor = border;
}

void UISlider::setValue(int val) {
    value = constrain(val, minValue, maxValue);
}

void UISlider::draw(TFT_Module& tft) {
    auto display = tft.getTFT();
    
    // Draw track background
    display->fillRoundRect(x, y, width, height, 5, trackColor);
    display->drawRoundRect(x, y, width, height, 5, borderColor);
    
    // Draw filled portion (bottom to thumb)
    int thumbY = valueToY(value);
    int fillHeight = (y + height) - thumbY;
    if (fillHeight > 0) {
        display->fillRoundRect(x + 2, thumbY, width - 4, fillHeight, 3, 0x04BF);  // Cyan fill
    }
    
    // Draw thumb (horizontal bar)
    int thumbHeight = 15;
    display->fillRoundRect(x - 2, thumbY - thumbHeight/2, width + 4, thumbHeight, 3, thumbColor);
    display->drawRoundRect(x - 2, thumbY - thumbHeight/2, width + 4, thumbHeight, 3, borderColor);
}

bool UISlider::handleTouch(int touchX, int touchY) {
    // Check if touch is within slider bounds (with some margin)
    if (touchX >= x - 10 && touchX <= x + width + 10 &&
        touchY >= y && touchY <= y + height) {
        
        value = yToValue(touchY);
        return true;
    }
    return false;
}

int UISlider::valueToY(int val) {
    // Map value to Y coordinate (inverted - high value at top)
    float ratio = (float)(val - minValue) / (float)(maxValue - minValue);
    return y + height - (int)(ratio * height);
}

int UISlider::yToValue(int touchY) {
    // Map Y coordinate to value (inverted)
    int relativeY = (y + height) - touchY;
    relativeY = constrain(relativeY, 0, height);
    float ratio = (float)relativeY / (float)height;
    int val = minValue + (int)(ratio * (maxValue - minValue));
    return constrain(val, minValue, maxValue);
}
