// =====================================================================
//  KidScreen.cpp - Kid screen implementation
// =====================================================================

#include "KidScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../utils/VS1053_Module.h"
#include "../utils/SD_Module.h"  
#include "../managers/MP3Player.h"  
#include <LovyanGFX.hpp>
#include <TJpg_Decoder.h>

// Add this global pointer (ugly but TJpg_Decoder needs it)
TFT_Module* globalTFT = nullptr;

// Add this callback function BEFORE the KidScreen class methods:
bool tftOutput(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    // This function is called by TJpg_Decoder to output decoded JPEG data
    // We'll set the target TFT in displayAlbumArt()
    extern TFT_Module* globalTFT;
    if (globalTFT && bitmap) {
        auto display = globalTFT->getTFT();
        display->pushImage(x, y, w, h, bitmap);
    }
    return true;
}

KidScreen::KidScreen(ScreenManager& manager, TFT_Module& tftModule, VS1053_Module& audio, SD_Module& sd)
    : BaseScreen(manager, tftModule),
      audioModule(audio),
      sdModule(sd),  // Add this line
      albumLoaded(false),
      isPlaying(false),
      prevButton(40, 240, 80, 60, "<<"),
      playPauseButton(200, 240, 80, 60, "||"),
      nextButton(360, 240, 80, 60, ">>"),
      backButton(10, 10, 100, 40, "Back"),
      volumeSlider(440, 80, 30, 200, 0, 100)
{
    currentAlbum[0] = '\0';
    
    // Customize button colors - bright and kid-friendly
    prevButton.setColors(0xF800, TFT_WHITE, TFT_WHITE);      // Red
    playPauseButton.setColors(0x07E0, TFT_WHITE, TFT_BLACK); // Green
    nextButton.setColors(0x001F, TFT_WHITE, TFT_WHITE);      // Blue
    backButton.setColors(0x632C, TFT_WHITE, TFT_WHITE);      // Gray
    
    // Set initial volume to 75%
    volumeSlider.setValue(75);
}

void KidScreen::begin() {
    if (!albumLoaded) {
        drawWaitingScreen();
    } else {
        drawPlaybackScreen();
    }
}

void KidScreen::drawWaitingScreen() {
    auto display = tft.getTFT();
    
    display->fillScreen(TFT_BLACK);
    
    // Animated NFC prompt
    display->setTextColor(TFT_CYAN);
    display->setTextDatum(middle_center);
    display->setTextSize(3);
    display->drawString("Place Your Card", 240, 120);
    display->drawString("on the Reader!", 240, 160);
    
    // Draw NFC icon placeholder
    display->drawCircle(240, 220, 40, TFT_GREEN);
    display->drawCircle(240, 220, 30, TFT_GREEN);
    display->drawCircle(240, 220, 20, TFT_GREEN);
    
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
    //display->fillRoundRect(90, 60, 300, 150, 10, TFT_PURPLE);
    //display->drawRoundRect(90, 60, 300, 150, 10, TFT_WHITE);
    
    // Draw "NOW PLAYING" indicator
    display->setTextSize(1);
    display->setTextColor(TFT_GREEN);
    display->drawString("NOW PLAYING", 240, 220);
    
    // Draw playback controls
    prevButton.draw(tft);
    playPauseButton.draw(tft);
    nextButton.draw(tft);
    backButton.draw(tft);
    
    // Draw volume slider on right side
    volumeSlider.draw(tft);
    
    // Draw volume labels
    display->setTextSize(1);
    display->setTextDatum(middle_center);
    display->setTextColor(TFT_WHITE);
    display->drawString("VOL", 455, 60);
    display->drawString(String(volumeSlider.getValue()).c_str(), 455, 290);
}

void KidScreen::update() {
    // Future: animate waiting screen, update playback position
}

void KidScreen::handleTouch(int x, int y) {
    // Check volume slider first (always available)
    
    Serial.printf("KidScreen got touch: (%d,%d)\n", x, y);
    Serial.printf("  Slider bounds: x=%d-%d, y=%d-%d\n", 
                  440-30, 440+30+30, 80-10, 80+200+10);
    Serial.printf("  PlayPause bounds: x=%d-%d, y=%d-%d\n",
                  200, 200+80, 240, 240+60);

    if (volumeSlider.handleTouch(x, y)) {
        int volume = volumeSlider.getValue();
        audioModule.setVolume(volume);
        volumeSlider.draw(tft);  // Redraw just the slider
        
        // Update volume percentage display
        auto display = tft.getTFT();
        display->fillRect(445, 280, 20, 20, TFT_BLACK);  // Clear old number
        display->setTextSize(1);
        display->setTextDatum(middle_center);
        display->setTextColor(TFT_WHITE);
        display->drawString(String(volume).c_str(), 455, 290);
        return;
    }
    
    // Back button always available
    if (backButton.hit(x, y)) {
        Serial.println("Returning to splash");
        clearAlbum();
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
                audioModule.stopPlayback();
            }
            
            begin();  // Redraw to update button
        }
        else if (nextButton.hit(x, y)) {
            Serial.println("Next track");
            // TODO: Implement next track
        }
    }
}

void KidScreen::playMP3FromSD() {
    extern MP3Player mp3Player;
    
    char mp3Path[128];
    if (!sdModule.getFirstMP3(mp3Path, sizeof(mp3Path))) {
        Serial.println("No MP3 found!");
        return;
    }
    
    mp3Player.play(mp3Path);
}

void KidScreen::showAlbum(const char* albumName) {
    strncpy(currentAlbum, albumName, sizeof(currentAlbum) - 1);
    currentAlbum[sizeof(currentAlbum) - 1] = '\0';
    
    albumLoaded = true;
    isPlaying = true;
    
    Serial.printf("Kid Screen: Loading album '%s'\n", albumName);
    
    drawPlaybackScreen();
    
    // Play test tone when album loads
    //audioModule.playTestTone(440);
    // Play first MP3 instead of tone

    // Display album art BEFORE starting music
    displayAlbumArt();
    

    playMP3FromSD();

}


void KidScreen::clearAlbum() {
    extern MP3Player mp3Player;
    mp3Player.stop();
    Serial.println("Kid Screen: Clearing album (NFC removed)");
    
    albumLoaded = false;
    isPlaying = false;
    currentAlbum[0] = '\0';
    
    // Stop playback
    audioModule.stopPlayback();
    
    // Return to splash
    screenManager.showSplash();
}

// Add this method to KidScreen class:
void KidScreen::displayAlbumArt() {
    // Get first album folder
    char albumPath[128];
    char mp3Path[128];
    Serial.println("=== DISPLAY ALBUM ART CALLED ===");
    if (!sdModule.getFirstMP3(mp3Path, sizeof(mp3Path))) {
        Serial.println("No albums found for art");
        return;
    }
    
    // Extract folder path from MP3 path (everything before last /)
    strncpy(albumPath, mp3Path, sizeof(albumPath));
    char* lastSlash = strrchr(albumPath, '/');
    if (lastSlash) {
        *lastSlash = '\0';  // Truncate at last slash
    }
    
    // Find album art in that folder
    char artPath[128];
    if (!sdModule.getAlbumArt(albumPath, artPath, sizeof(artPath))) {
        Serial.println("No album art found");
        return;
    }
    
    // Set up TJpg_Decoder
    TJpgDec.setJpgScale(1);  // 1:1 scale
    TJpgDec.setCallback(tftOutput);
    
    // Set global TFT pointer for callback
    globalTFT = &tft;
    
    // Open and decode JPEG
    if (!sdModule.openFile(artPath)) {
        Serial.println("Failed to open album art");
        return;
    }
    
    // Read entire file into buffer (album art should be small)
    uint8_t* buffer = new uint8_t[50000];  // 50KB buffer
    size_t totalRead = 0;
    size_t bytesRead;
    
    while ((bytesRead = sdModule.readChunk(buffer + totalRead, 512)) > 0) {
        totalRead += bytesRead;
        if (totalRead >= 50000) break;  // Safety limit
    }
    
    sdModule.closeFile();
    
    if (totalRead > 0) {
        Serial.printf("Decoding %d bytes of JPEG...\n", totalRead);
        
        // Get JPEG dimensions
        uint16_t w, h;
        TJpgDec.getJpgSize(&w, &h, buffer, totalRead);
        Serial.printf("JPEG size: %dx%d\n", w, h);
        
        // Center in the 300x150 box
        int16_t x = 90 + (300 - w) / 2;
        int16_t y = 60 + (150 - h) / 2;
        
        TJpgDec.setJpgScale(1);  // No scaling
        TJpgDec.setSwapBytes(true);
        TJpgDec.drawJpg(x, y, buffer, totalRead);
    }
    
    delete[] buffer;
    globalTFT = nullptr;
    
    Serial.println("Album art displayed!");
}