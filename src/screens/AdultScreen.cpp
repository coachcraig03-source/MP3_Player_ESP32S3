// =====================================================================
//  AdultScreen.cpp - Adult screen implementation
// =====================================================================

#include "AdultScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>

AdultScreen::AdultScreen(ScreenManager& manager, TFT_Module& tftModule)
    : BaseScreen(manager, tftModule),
      albumList(nullptr),
      albumCount(0),
      scrollOffset(0),
      selectedAlbum(-1),
      isPlaying(false),
      backButton(10, 10, 80, 35, "Back"),
      playPauseButton(100, 270, 80, 40, "Play"),
      prevButton(200, 270, 80, 40, "<<"),
      nextButton(300, 270, 80, 40, ">>")
{
    backButton.setColors(0x632C, TFT_WHITE, TFT_WHITE);
    playPauseButton.setColors(0x07E0, TFT_WHITE, TFT_BLACK);
    prevButton.setColors(0xF800, TFT_WHITE, TFT_WHITE);
    nextButton.setColors(0x001F, TFT_WHITE, TFT_WHITE);
}

void AdultScreen::begin() {
    auto display = tft.getTFT();
    
    display->fillScreen(TFT_BLACK);
    
    // Header
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_left);
    display->setTextSize(2);
    display->drawString("Album Library", 100, 15);
    
    backButton.draw(tft);
    
    if (albumCount == 0) {
        // No albums loaded yet
        display->setTextDatum(middle_center);
        display->setTextColor(TFT_YELLOW);
        display->drawString("No Albums Found", 240, 120);
        display->setTextSize(1);
        display->setTextColor(TFT_DARKGREY);  // Change from TFT_GRAY
        display->drawString("Insert SD card with MP3 files", 240, 150);
    } else {
        drawAlbumList();
        
        if (isPlaying) {
            drawPlaybackInfo();
        }
    }
}

void AdultScreen::drawAlbumList() {
    auto display = tft.getTFT();
    
    // Draw scrollable list area
    int listY = 60;
    int itemHeight = 35;
    
    display->setTextSize(2);
    display->setTextDatum(top_left);
    
    for (int i = 0; i < MAX_VISIBLE_ALBUMS && (scrollOffset + i) < albumCount; i++) {
        int albumIndex = scrollOffset + i;
        int y = listY + (i * itemHeight);
        
        // Highlight selected album
        if (albumIndex == selectedAlbum) {
            display->fillRect(10, y, 460, itemHeight - 2, 0x1F7C);  // Dark blue
        }
        
        // Draw album name
        display->setTextColor(albumIndex == selectedAlbum ? TFT_YELLOW : TFT_WHITE);
        display->drawString(albumList[albumIndex], 20, y + 5);
        
        // Draw separator
        display->drawLine(10, y + itemHeight - 1, 470, y + itemHeight - 1, TFT_DARKGREY);
    }
    
    // Scroll indicators
    if (scrollOffset > 0) {
        display->fillTriangle(460, 65, 450, 75, 470, 75, TFT_CYAN);  // Up arrow
    }
    if (scrollOffset + MAX_VISIBLE_ALBUMS < albumCount) {
        display->fillTriangle(460, 255, 450, 245, 470, 245, TFT_CYAN);  // Down arrow
    }
}

void AdultScreen::drawPlaybackInfo() {
    // Draw playback controls
    playPauseButton.draw(tft);
    prevButton.draw(tft);
    nextButton.draw(tft);
}

void AdultScreen::update() {
    // Future: update playback time, visualizer
}

void AdultScreen::handleTouch(int x, int y) {
    // Back button
    if (backButton.hit(x, y)) {
        Serial.println("Returning to splash");
        screenManager.showSplash();
        return;
    }
    
    // Playback controls (if playing)
    if (isPlaying) {
        if (playPauseButton.hit(x, y)) {
            isPlaying = !isPlaying;
            playPauseButton.setLabel(isPlaying ? "Pause" : "Play");
            begin();  // Redraw
            Serial.println(isPlaying ? "Resumed" : "Paused");
            return;
        }
        if (prevButton.hit(x, y)) {
            Serial.println("Previous track");
            // TODO: Previous track
            return;
        }
        if (nextButton.hit(x, y)) {
            Serial.println("Next track");
            // TODO: Next track
            return;
        }
    }
    
    // Album list touch
    if (y >= 60 && y < 270) {
        int albumIndex = getTouchedAlbumIndex(y);
        if (albumIndex >= 0 && albumIndex < albumCount) {
            playAlbum(albumIndex);
        }
    }
    
    // Scroll arrows
    if (x > 440 && x < 480) {
        if (y > 60 && y < 80 && scrollOffset > 0) {
            scrollOffset--;
            begin();  // Redraw list
        }
        else if (y > 240 && y < 260 && scrollOffset + MAX_VISIBLE_ALBUMS < albumCount) {
            scrollOffset++;
            begin();  // Redraw list
        }
    }
}

int AdultScreen::getTouchedAlbumIndex(int y) {
    int listY = 60;
    int itemHeight = 35;
    int relativeY = y - listY;
    int index = scrollOffset + (relativeY / itemHeight);
    
    if (index >= 0 && index < albumCount) {
        return index;
    }
    return -1;
}

void AdultScreen::setAlbumList(const char** albums, int count) {
    albumList = albums;
    albumCount = count;
    scrollOffset = 0;
    
    Serial.printf("Adult Screen: Loaded %d albums\n", count);
}

void AdultScreen::playAlbum(int index) {
    if (index < 0 || index >= albumCount) return;
    
    selectedAlbum = index;
    isPlaying = true;
    
    Serial.printf("Adult Screen: Playing album '%s'\n", albumList[index]);
    
    // TODO: Start playback
    
    begin();  // Redraw with selection
}
