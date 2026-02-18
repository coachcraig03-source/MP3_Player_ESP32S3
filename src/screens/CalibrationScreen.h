// =====================================================================
//  CalibrationScreen.h - Touch screen calibration
// =====================================================================

#ifndef CALIBRATION_SCREEN_H
#define CALIBRATION_SCREEN_H

#include <Arduino.h>  // Add this for uint32_t
#include "../managers/BaseScreen.h"

class CalibrationScreen : public BaseScreen {
public:
    CalibrationScreen(ScreenManager& manager, TFT_Module& tft);

    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;
    
    // Start calibration process
    void startCalibration();

private:
    void drawCrosshair(int x, int y, uint32_t color);
    void calculateCalibration();
    void saveCalibration();
    
    // Calibration points (screen coordinates)
    static const int NUM_POINTS = 4;
    struct Point {
        int x, y;
    };
    
    Point screenPoints[NUM_POINTS] = {
        {40, 40},      // Top-left
        {440, 40},     // Top-right
        {440, 280},    // Bottom-right
        {40, 280}      // Bottom-left
    };
    
    Point touchPoints[NUM_POINTS];
    
    int currentPoint;
    bool calibrating;
    bool calibrationComplete;
};

#endif // CALIBRATION_SCREEN_H
