// =====================================================================
//  TouchCalibration.h - Touch coordinate transformation
// =====================================================================

#ifndef TOUCH_CALIBRATION_H
#define TOUCH_CALIBRATION_H

#include <Arduino.h>
#include <Preferences.h>

class TouchCalibration {
public:
    // Singleton instance
    static TouchCalibration& getInstance() {
        static TouchCalibration instance;
        return instance;
    }
    
    // Set calibration values
    void setCalibration(float scaleX, float scaleY, float offsetX, float offsetY);
    
    // Transform raw touch coordinates to screen coordinates
    void transform(int rawX, int rawY, int& screenX, int& screenY);
    
    // Save/load from preferences
    void saveToPreferences();
    void loadFromPreferences();
    
    // Check if calibrated
    bool isCalibrated() const { return calibrated; }
    
    // Reset to defaults (no calibration)
    void reset();

private:
    TouchCalibration();  // Private constructor for singleton
    
    float scaleX;
    float scaleY;
    float offsetX;
    float offsetY;
    bool calibrated;
    
    Preferences prefs;
};

#endif // TOUCH_CALIBRATION_H
