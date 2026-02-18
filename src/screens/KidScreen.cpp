// =====================================================================
//  KidScreen.cpp - Kid screen implementation
// =====================================================================

#include "KidScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../utils/VS1053_Module.h"
#include <LovyanGFX.hpp>

KidScreen::KidScreen(ScreenManager& manager, TFT_Module& tftModule, VS1053_Module& audio)
    : BaseScreen(manager, tftModule),
      audioModule(audio),
      albumLoaded(false),
      isPlaying(false),
      prevButton(40, 240, 80, 60, "<<"),
      playPauseButton(200, 240, 80, 60, "||"),
      nextButton(360, 240, 80, 60, ">>"),
      backButton(10, 10, 100, 40, "Back")
{
    currentAlbum[0] = '\0';
    
    // Customize button colors - bright and kid-friendly
    prevButton.setColors(0xF800, TFT_WHITE, TFT_WHITE);      // Red
    playPauseButton.setColors(0x07E0, TFT_WHITE, TFT_BLACK); // Green
    nextButton.setColors(0x001F, TFT_WHITE, TFT_WHITE);      // Blue
    backButton.setColors(0x632C, TFT_WHITE, TFT_WHITE);      // Gray
}

void KidScreen::begin() {
    if (albumLoaded) {
        drawPlaybackScreen();
    } else {
        drawWaitingScreen();
    }
}

void KidScreen::drawWaitingScreen() {
    auto display = tft.getTFT();
    
    display->fillScreen(TFT_BLACK);
    
    // Draw fun waiting message
    display->setTextColor(TFT_YELLOW);
    display->setTextDatum(middle_center);
    display->setTextSize(3);
    display->drawString("Place Your Card", 240, 100);
    display->drawString("on the Reader!", 240, 140);
    
    // Draw animated NFC icon area (placeholder for now)
    display->drawCircle(240, 200, 40, TFT_CYAN);
    display->drawCircle(240, 200, 30, TFT_CYAN);
    display->drawCircle(240, 200, 20, TFT_CYAN);
    
    // Draw back button
    backButton.draw(tft);
}

void KidScreen::drawPlaybackScreen() {
    auto display = tft.getTFT();
    
    display->fillScreen(TFT_BLACK);
    
    // Album name at top
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->setTextSize(2);
    display->drawString(currentAlbum, 240, 20);
    
    // Album art placeholder (colorful rectangle for now)
    display->fillRoundRect(90, 60, 300, 150, 10, TFT_PURPLE);
    display->drawRoundRect(90, 60, 300, 150, 10, TFT_WHITE);
    
    // Draw "NOW PLAYING" indicator
    display->setTextSize(1);
    display->setTextColor(TFT_GREEN);
    display->drawString("NOW PLAYING", 240, 220);
    
    // Draw playback controls
    prevButton.draw(tft);
    playPauseButton.draw(tft);
    nextButton.draw(tft);
    backButton.draw(tft);
}

void KidScreen::update() {
    // Future: animate waiting screen, update playback position
}

void KidScreen::handleTouch(int x, int y) {
    // Back button always available
    if (backButton.hit(x, y)) {
        Serial.println("Returning to splash");
        clearAlbum();  // Stop playback
        screenManager.showSplash();
        return;
    }
    
    // Playback controls only available when album is loaded
    if (albumLoaded) {
        if (prevButton.hit(x, y)) {
            Serial.println("Previous track");
            // TODO: Implement previous track
        }
        else if (playPauseButton.hit(x, y)) {
            isPlaying = !isPlaying;
            Serial.println(isPlaying ? "Playing" : "Paused");
            playPauseButton.setLabel(isPlaying ? "||" : ">");
            
            if (!isPlaying) {
                // Stop playback when paused
                audioModule.stopPlayback();
            }
            
            begin();  // Redraw to update button
            // TODO: Implement play/pause
        }
        else if (nextButton.hit(x, y)) {
            Serial.println("Next track");
            // TODO: Implement next track
        }
    }
}

void KidScreen::showAlbum(const char* albumName) {
    strncpy(currentAlbum, albumName, sizeof(currentAlbum) - 1);
    currentAlbum[sizeof(currentAlbum) - 1] = '\0';
    
    albumLoaded = true;
    isPlaying = true;
    
    Serial.printf("Kid Screen: Loading album '%s'\n", albumName);
    
    drawPlaybackScreen();
    
    // Play test tone when album loads
 audioModule.playTestTone(440);  // No duration
}

void KidScreen::clearAlbum() {
    Serial.println("Kid Screen: Clearing album (NFC removed)");
    
    albumLoaded = false;
    isPlaying = false;
    currentAlbum[0] = '\0';
    
    // Stop playback
    audioModule.stopPlayback();
    
    // Return to splash
    screenManager.showSplash();
}
