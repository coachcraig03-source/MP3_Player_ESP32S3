// =====================================================================
//  CalibrationScreen.cpp - Touch calibration implementation
// =====================================================================

#include "CalibrationScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../utils/TouchCalibration.h"
#include <LovyanGFX.hpp>

CalibrationScreen::CalibrationScreen(ScreenManager& manager, TFT_Module& tftModule)
    : BaseScreen(manager, tftModule),
      currentPoint(0),
      calibrating(false),
      calibrationComplete(false)
{
}

void CalibrationScreen::begin() {
    auto display = tft.getTFT();
    
    display->fillScreen(TFT_BLACK);
    
    if (!calibrating && !calibrationComplete) {
        // Show instructions
        display->setTextColor(TFT_WHITE);
        display->setTextDatum(middle_center);
        display->setTextSize(2);
        display->drawString("Touch Screen", 240, 100);
        display->drawString("Calibration", 240, 130);
        
        display->setTextSize(1);
        display->setTextColor(TFT_CYAN);
        display->drawString("Tap each crosshair", 240, 180);
        display->drawString("as it appears", 240, 200);
        
        display->setTextColor(TFT_YELLOW);
        display->drawString("Touch anywhere to start", 240, 250);
    }
    else if (calibrating) {
        // Draw current calibration point
        display->fillScreen(TFT_BLACK);
        display->setTextColor(TFT_WHITE);
        display->setTextDatum(top_center);
        display->setTextSize(2);
        display->drawString("Tap the crosshair", 240, 20);
        
        display->setTextSize(1);
        display->setTextColor(TFT_CYAN);
        char buf[32];
        sprintf(buf, "Point %d of %d", currentPoint + 1, NUM_POINTS);
        display->drawString(buf, 240, 50);
        
        // Draw crosshair at current point
        drawCrosshair(screenPoints[currentPoint].x, screenPoints[currentPoint].y, TFT_RED);
    }
    else if (calibrationComplete) {
        // Show completion message
        display->fillScreen(TFT_BLACK);
        display->setTextColor(TFT_GREEN);
        display->setTextDatum(middle_center);
        display->setTextSize(3);
        display->drawString("Calibration", 240, 120);
        display->drawString("Complete!", 240, 160);
        
        display->setTextSize(1);
        display->setTextColor(TFT_WHITE);
        display->drawString("Touch anywhere to continue", 240, 220);
    }
}

void CalibrationScreen::startCalibration() {
    currentPoint = 0;
    calibrating = true;
    calibrationComplete = false;
    begin();
}

void CalibrationScreen::update() {
    // No animations needed
}

void CalibrationScreen::handleTouch(int x, int y) {
    if (!calibrating && !calibrationComplete) {
        // Start calibration
        startCalibration();
    }
    else if (calibrating) {
        // Record touch point
        Serial.printf("Calibration point %d: Touch=(%d,%d) Screen=(%d,%d)\n",
                     currentPoint, x, y,
                     screenPoints[currentPoint].x,
                     screenPoints[currentPoint].y);
        
        touchPoints[currentPoint].x = x;
        touchPoints[currentPoint].y = y;
        
        currentPoint++;
        
        if (currentPoint >= NUM_POINTS) {
            // Calibration complete
            calibrating = false;
            calibrationComplete = true;
            calculateCalibration();
            saveCalibration();
            begin();
        } else {
            // Show next point
            begin();
        }
    }
    else if (calibrationComplete) {
        // Return to splash
        screenManager.showSplash();
    }
}

void CalibrationScreen::drawCrosshair(int x, int y, uint32_t color) {
    auto display = tft.getTFT();
    
    // Draw crosshair
    int size = 20;
    display->drawLine(x - size, y, x + size, y, color);
    display->drawLine(x, y - size, x, y + size, color);
    display->drawCircle(x, y, 5, color);
    display->drawCircle(x, y, 10, color);
}

void CalibrationScreen::calculateCalibration() {
    // Touch coordinates are rotated 90° - axes are swapped
    // Touch X → Screen Y, Touch Y → Screen X
    
    // Touch X maps to Screen Y (use points 0 and 3 - vertical on screen)
    float scaleY = (float)(screenPoints[3].y - screenPoints[0].y) / 
                   (float)(touchPoints[3].x - touchPoints[0].x);
    
    // Touch Y maps to Screen X (use points 0 and 1 - horizontal on screen)
    float scaleX = (float)(screenPoints[1].x - screenPoints[0].x) / 
                   (float)(touchPoints[1].y - touchPoints[0].y);
    
    float offsetX = screenPoints[0].x - (touchPoints[0].y * scaleX);
    float offsetY = screenPoints[0].y - (touchPoints[0].x * scaleY);
    
    Serial.println("=== Calibration Results ===");
    Serial.println("Touch points:");
    for (int i = 0; i < NUM_POINTS; i++) {
        Serial.printf("  Point %d: Touch=(%d,%d) Screen=(%d,%d)\n",
                     i, touchPoints[i].x, touchPoints[i].y,
                     screenPoints[i].x, screenPoints[i].y);
    }
    Serial.printf("Scale X: %.4f  Offset X: %.2f\n", scaleX, offsetX);
    Serial.printf("Scale Y: %.4f  Offset Y: %.2f\n", scaleY, offsetY);
    
    // Sanity check: scales should be between -3.0 and 3.0
    if (fabs(scaleX) > 3.0 || fabs(scaleY) > 3.0) {
        Serial.println("CALIBRATION REJECTED: Scale values out of range!");
        Serial.println("Keeping previous calibration. Please try again.");
        calibrationComplete = false;
        calibrating = false;
        begin();
        return;
    }
    
    // Verify calculation with swapped formula
    int testY = (int)(touchPoints[2].x * scaleY + offsetY);
    int testX = (int)(touchPoints[2].y * scaleX + offsetX);
    Serial.printf("Verification (point 2): Expected=(%d,%d) Calculated=(%d,%d)\n",
                 screenPoints[2].x, screenPoints[2].y, testX, testY);
    
    // Sanity check verification - should be within 50 pixels
    int errorX = abs(testX - screenPoints[2].x);
    int errorY = abs(testY - screenPoints[2].y);
    if (errorX > 50 || errorY > 50) {
        Serial.printf("CALIBRATION REJECTED: Verification error too large! (X:%d Y:%d)\n", errorX, errorY);
        Serial.println("Keeping previous calibration. Please try again.");
        calibrationComplete = false;
        calibrating = false;
        begin();
        return;
    }
    
    Serial.println("Calibration ACCEPTED!");
    TouchCalibration::getInstance().setCalibration(scaleX, scaleY, offsetX, offsetY);
}

void CalibrationScreen::saveCalibration() {
    TouchCalibration::getInstance().saveToPreferences();
    Serial.println("Calibration saved!");
}
