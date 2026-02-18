// =====================================================================
//  TouchCalibration.cpp - Touch calibration implementation
// =====================================================================

#include "TouchCalibration.h"

TouchCalibration::TouchCalibration()
    : scaleX(1.0), scaleY(1.0), offsetX(0.0), offsetY(0.0), calibrated(false)
{
    loadFromPreferences();
}

void TouchCalibration::setCalibration(float sx, float sy, float ox, float oy) {
    scaleX = sx;
    scaleY = sy;
    offsetX = ox;
    offsetY = oy;
    calibrated = true;
    
    Serial.println("TouchCalibration: Values updated");
}

void TouchCalibration::transform(int rawX, int rawY, int& screenX, int& screenY) {
    if (calibrated) {
        // SWAP: touch X maps to screen Y, touch Y maps to screen X
        screenY = (int)(rawX * scaleY + offsetY);  // Changed: scaleX -> scaleY, offsetX -> offsetY
        screenX = (int)(rawY * scaleX + offsetX);  // Changed: scaleY -> scaleX, offsetY -> offsetX
    } else {
        screenX = rawX;
        screenY = rawY;
    }
}

void TouchCalibration::saveToPreferences() {
    prefs.begin("touch_cal", false);
    prefs.putFloat("scaleX", scaleX);
    prefs.putFloat("scaleY", scaleY);
    prefs.putFloat("offsetX", offsetX);
    prefs.putFloat("offsetY", offsetY);
    prefs.putBool("calibrated", calibrated);
    prefs.end();
    
    Serial.println("TouchCalibration: Saved to preferences");
}

void TouchCalibration::loadFromPreferences() {
    prefs.begin("touch_cal", true);  // Read-only
    
    if (prefs.isKey("calibrated")) {
        scaleX = prefs.getFloat("scaleX", 1.0);
        scaleY = prefs.getFloat("scaleY", 1.0);
        offsetX = prefs.getFloat("offsetX", 0.0);
        offsetY = prefs.getFloat("offsetY", 0.0);
        calibrated = prefs.getBool("calibrated", false);
        
        Serial.println("TouchCalibration: Loaded from preferences");
        Serial.printf("  Scale: %.4f, %.4f\n", scaleX, scaleY);
        Serial.printf("  Offset: %.2f, %.2f\n", offsetX, offsetY);
    } else {
        Serial.println("TouchCalibration: No saved calibration found");
    }
    
    prefs.end();
}

void TouchCalibration::reset() {
    scaleX = 1.0;
    scaleY = 1.0;
    offsetX = 0.0;
    offsetY = 0.0;
    calibrated = false;
    
    prefs.begin("touch_cal", false);
    prefs.clear();
    prefs.end();
    
    Serial.println("TouchCalibration: Reset to defaults");
}
