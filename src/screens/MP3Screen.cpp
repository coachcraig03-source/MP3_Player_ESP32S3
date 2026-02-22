// =====================================================================
//  MP3Screen.cpp - MP3 Library Browser Implementation
// =====================================================================

#include "MP3Screen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../utils/SD_Module.h"
#include "../utils/VS1053_Module.h"
#include "../managers/MP3Player.h"  
#include <LovyanGFX.hpp>
#include <SdFat.h>
#include <TJpg_Decoder.h>
//#include <lgfx/v1/misc/fonts/FreeSans9pt7b.hpp>

#define LIST_X 220
#define LIST_Y 60
#define LIST_W 200
#define LIST_H 200
#define ITEM_HEIGHT 18
#define MAX_VISIBLE 10

// Global for JPEG decoder callback
TFT_Module* globalTFT_MP3 = nullptr;



bool tftOutput_MP3(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (!globalTFT_MP3) {
        Serial.println("ERROR: globalTFT_MP3 is NULL!");
        return false;
    }
    
    if (!bitmap) {
        Serial.println("ERROR: bitmap is NULL!");
        return false;
    }
    
    auto display = globalTFT_MP3->getTFT();
    if (!display) {
        Serial.println("ERROR: display is NULL!");
        return false;
    }
    
    display->pushImage(x, y, w, h, bitmap);
    return true;
}

MP3Screen::MP3Screen(ScreenManager& manager, TFT_Module& tftModule, SD_Module& sd, VS1053_Module& audio)
    : BaseScreen(manager, tftModule),
      sdModule(sd),
      audioModule(audio),
      albumCount(0),
      selectedAlbum(-1),
      trackCount(0),
      selectedTrack(-1),
      inAlbumView(true),
      scrollOffset(0),
      isPlaying(false),
      backButton(10, 10, 80, 40, "Back"),
      prevButton(40, 270, 60, 40, "<<"),
      playPauseButton(120, 270, 80, 40, "Play"),
      nextButton(220, 270, 60, 40, ">>"),
      volumeSlider(440, 60, 30, 200, 0, 100)
{
    backButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    prevButton.setColors(TFT_BLUE, TFT_WHITE, TFT_WHITE);
    playPauseButton.setColors(TFT_GREEN, TFT_BLACK, TFT_BLACK);
    nextButton.setColors(TFT_BLUE, TFT_WHITE, TFT_WHITE);
    volumeSlider.setValue(75);
}

void MP3Screen::begin() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);
    
    // Load albums from SD on first entry
    if (albumCount == 0) {
        loadAlbumsFromSD();
    }
    
    drawLayout();
}

void MP3Screen::drawLayout() {
    auto display = tft.getTFT();
    
    // Title
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_left);
    display->setTextSize(1);
    display->drawString(inAlbumView ? "Albums" : "Tracks", 200, 15);
    
    // Back button
    backButton.draw(tft);
    
    // Album art area (left side)
    display->drawRect(10, 60, 200, 200, TFT_WHITE);
   // drawAlbumArt();
    
    // CLEAR list area before drawing
    display->fillRect(LIST_X + 1, LIST_Y + 1, LIST_W - 2, LIST_H - 2, TFT_BLACK);
    
    // List area border
    display->drawRect(LIST_X, LIST_Y, LIST_W, LIST_H, TFT_WHITE);

    if (inAlbumView) {
        drawAlbumList();
    } else {
        drawTrackList();
    }
    
    // Volume slider
    volumeSlider.draw(tft);
    display->setTextSize(1);
    display->setTextDatum(top_center);
    display->drawString("VOL", 455, 40);
    
    // Playback controls
    prevButton.draw(tft);
    playPauseButton.draw(tft);
    nextButton.draw(tft);
}

void MP3Screen::loadAlbumsFromSD() {
    Serial.println("MP3Screen: Loading albums from SD...");
    
    if (!sdModule.isInitialized()) {
        Serial.println("MP3Screen: SD not initialized");
        return;
    }
    
    // Open root directory
    extern SdFat sd;  // Access SD from SD_Module
    FsFile root;
    if (!root.open("/Music")) {
        Serial.println("MP3Screen: Failed to open /Music");
        return;
    }
    
    albumCount = 0;
    FsFile dir;
    
    while (dir.openNext(&root, O_RDONLY) && albumCount < 50) {
        if (dir.isDirectory()) {
            char name[64];
            dir.getName(name, sizeof(name));
            
            // Skip system directories
            if (name[0] != '.' && strcmp(name, "System Volume Information") != 0) {
                strncpy(albumNames[albumCount], name, sizeof(albumNames[0]) - 1);
                albumNames[albumCount][sizeof(albumNames[0]) - 1] = '\0';
                albumCount++;
                Serial.printf("  Found album: %s\n", name);
            }
        }
        dir.close();
    }
    
    root.close();
    Serial.printf("MP3Screen: Loaded %d albums\n", albumCount);
}

void MP3Screen::drawAlbumList() {
    auto display = tft.getTFT();
    
    display->setTextSize(1);
    display->setTextDatum(top_left);
    
    for (int i = 0; i < MAX_VISIBLE && (scrollOffset + i) < albumCount; i++) {
        int albumIndex = scrollOffset + i;
        int y = LIST_Y + 5 + (i * ITEM_HEIGHT);
        
        // Highlight selected
        if (albumIndex == selectedAlbum) {
            display->fillRect(LIST_X + 2, y - 2, LIST_W - 4, ITEM_HEIGHT, TFT_DARKGREY);
        }
        
        // Draw album name (truncate if needed)
        display->setTextColor(albumIndex == selectedAlbum ? TFT_YELLOW : TFT_WHITE);
        String albumName = String(albumNames[albumIndex]);
        if (albumName.length() > 30) {
            albumName = albumName.substring(0, 15) + "...";
        }
        display->drawString(albumName, LIST_X + 5, y);
    }
    
    // Scroll arrows
    display->setTextSize(3);
    display->setTextDatum(middle_center);
    if (scrollOffset > 0) {
        display->setTextColor(TFT_CYAN);
        display->drawString("^", LIST_X + LIST_W + 15, LIST_Y + 20);
    }
    if (scrollOffset + MAX_VISIBLE < albumCount) {
        display->setTextColor(TFT_CYAN);
        display->drawString("v", LIST_X + LIST_W + 15, LIST_Y + LIST_H - 20);
    }
}

void MP3Screen::drawTrackList() {
    auto display = tft.getTFT();
    
    display->setTextSize(1);
    display->setTextDatum(top_left);
    
    for (int i = 0; i < MAX_VISIBLE && (scrollOffset + i) < trackCount; i++) {
        int trackIndex = scrollOffset + i;
        int y = LIST_Y + 5 + (i * ITEM_HEIGHT);
        
        // Highlight selected
        if (trackIndex == selectedTrack) {
            display->fillRect(LIST_X + 2, y - 2, LIST_W - 4, ITEM_HEIGHT, TFT_DARKGREY);
        }
        
        // Draw track name
        display->setTextColor(trackIndex == selectedTrack ? TFT_YELLOW : TFT_WHITE);
        String trackName = String(trackNames[trackIndex]);
        if (trackName.length() > 18) {
            trackName = trackName.substring(0, 15) + "...";
        }
        display->drawString(trackName, LIST_X + 5, y);
    }
    
    // Scroll arrows
    display->setTextSize(3);
    display->setTextDatum(middle_center);
    if (scrollOffset > 0) {
        display->setTextColor(TFT_CYAN);
        display->drawString("^", LIST_X + LIST_W + 15, LIST_Y + 20);
    }
    if (scrollOffset + MAX_VISIBLE < trackCount) {
        display->setTextColor(TFT_CYAN);
        display->drawString("v", LIST_X + LIST_W + 15, LIST_Y + LIST_H - 20);
    }
}

void MP3Screen::drawAlbumArt() {
    auto display = tft.getTFT();
    extern MP3Player mp3Player;
    
    mp3Player.stop();
    delay(100);

    if (selectedAlbum < 0) {
        // Placeholder
        display->setTextSize(1);
        display->setTextDatum(middle_center);
        display->setTextColor(TFT_DARKGREY);
        display->drawString("Album Art", 110, 160);
        return;
    }
    
    // Find folder.jpg in selected album
    char artPath[128];
    snprintf(artPath, sizeof(artPath), "/Music/%s/folder.jpg", albumNames[selectedAlbum]);  // ADD /Music/
    
    if (!sdModule.openFile(artPath)) {
        return;  // No art found
    }
    
    // Read JPEG
    uint8_t* buffer = new uint8_t[50000];
    size_t totalRead = 0;
    size_t bytesRead;
    
    while ((bytesRead = sdModule.readChunk(buffer + totalRead, 512)) > 0) {
        totalRead += bytesRead;
        if (totalRead >= 50000) break;
    }
    
    sdModule.closeFile();
    
    if (totalRead > 0) {
        TJpgDec.setCallback(tftOutput_MP3);
        TJpgDec.setSwapBytes(true);
        globalTFT_MP3 = &tft;
        
        // Draw at 10,60 (200x200 box)
        // Right before TJpgDec.drawJpg():
        globalTFT_MP3 = &tft;  // Make SURE this is set

        TJpgDec.drawJpg(10, 60, buffer, totalRead);
        
        globalTFT_MP3 = nullptr;
    }
    
    delete[] buffer;
}

void MP3Screen::selectAlbum(int index) {
    extern MP3Player mp3Player;
    
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

    if (index < 0 || index >= albumCount) return;

    //mp3Player.stop();
    //delay(100);
    
    // Stop and reset everything first
    mp3Player.stop();
    delay(200);
    
    // Reset VS1053 to clear decoder state (especially after WMA)
    audioModule.softReset();
    delay(100);

    selectedAlbum = index;
    Serial.printf("MP3Screen: Selected album '%s'\n", albumNames[index]);
    
    // Load album art FIRST (before starting playback to avoid SPI conflict)
    //drawAlbumArt();
    //delay(100);  // Let SPI settle
    
    // Load tracks from this album folder
    trackCount = 0;
    scrollOffset = 0;
    
    char folderPath[128];
    snprintf(folderPath, sizeof(folderPath), "/Music/%s", albumNames[index]);  // ADD /Music/
   
    extern SdFat sd;
    FsFile albumDir;
    if (!albumDir.open(folderPath)) {
        Serial.println("Failed to open album folder");
        return;
    }
    
    FsFile file;
    while (file.openNext(&albumDir, O_RDONLY) && trackCount < 100) {
        char name[64];
        file.getName(name, sizeof(name));
        
        if (strstr(name, ".mp3") || strstr(name, ".MP3") || 
            strstr(name, ".wma") || strstr(name, ".WMA")) {
            strncpy(trackNames[trackCount], name, sizeof(trackNames[0]) - 1);
            trackNames[trackCount][sizeof(trackNames[0]) - 1] = '\0';
            trackCount++;
        }
        
        file.close();
    }
    albumDir.close();

    // Load album art FIRST (before starting playback to avoid SPI conflict)
    drawAlbumArt();
    delay(100);  // Let SPI settle
    
    Serial.printf("Loaded %d tracks\n", trackCount);

    // Auto-play first track (art already loaded)
    if (trackCount > 0) {
        selectedTrack = 0;
        playTrack(0);
    }
    
    // Switch to track view
    inAlbumView = false;
    drawLayout();  // Redraw with tracks, art already visible
}

void MP3Screen::selectTrack(int index) {
    if (index < 0 || index >= trackCount) return;
    
    selectedTrack = index;
    Serial.printf("MP3Screen: Selected track '%s'\n", trackNames[index]);
    
    // TODO: Start playback
}

void MP3Screen::scrollList(int direction) {
    int maxItems = inAlbumView ? albumCount : trackCount;
    
    scrollOffset += direction;
    if (scrollOffset < 0) scrollOffset = 0;
    if (scrollOffset + MAX_VISIBLE > maxItems) {
        scrollOffset = max(0, maxItems - MAX_VISIBLE);
    }
    
    drawLayout();
}

void MP3Screen::update() {
    // Future: update playback time, etc.
}

void MP3Screen::handleTouch(int x, int y) {
    extern MP3Player mp3Player;
    // Back button
    if (backButton.hit(x, y)) {
        if (!inAlbumView) {
            // Go back to album list
            mp3Player.stop();
            delay(100);
            inAlbumView = true;
            scrollOffset = 0;
            drawLayout();
        } else {
            // Go back to splash
            mp3Player.stop();
            delay(300);
            screenManager.showSplash();
        }
        return;
    }

    if (playPauseButton.hit(x, y)) {
        isPlaying = !isPlaying;
        playPauseButton.setLabel(isPlaying ? "Pause" : "Play");
        playPauseButton.draw(tft);
        Serial.println(isPlaying ? "Playing" : "Paused");
        return;
    }

    if (nextButton.hit(x, y)) {
        Serial.println("Next track");
        nextTrack();  // Use the nextTrack() method we just created
        return;
    }

    if (prevButton.hit(x, y)) {
    Serial.println("Previous track");
    selectedTrack--;
    if (selectedTrack < 0) {
        selectedTrack = trackCount - 1;  // Wrap to last track
    }
    playTrack(selectedTrack);
    drawLayout();
    return;
}
    
    // Volume slider
    if (volumeSlider.handleTouch(x, y)) {
        int volume = volumeSlider.getValue();
        audioModule.setVolume(volume);
        volumeSlider.draw(tft);
        return;
    }
    
    // Playback controls
    if (prevButton.hit(x, y)) {
        Serial.println("Previous track");
        // TODO: Previous
        return;
    }
    if (playPauseButton.hit(x, y)) {
        isPlaying = !isPlaying;
        playPauseButton.setLabel(isPlaying ? "Pause" : "Play");
        playPauseButton.draw(tft);
        Serial.println(isPlaying ? "Playing" : "Paused");
        return;
    }
    if (nextButton.hit(x, y)) {
        Serial.println("Next track");
        // TODO: Next
        return;
    }
    
    // List area
    if (x >= LIST_X && x < LIST_X + LIST_W && y >= LIST_Y && y < LIST_Y + LIST_H) {
        int relativeY = y - LIST_Y - 5;
        int index = scrollOffset + (relativeY / ITEM_HEIGHT);
        
        if (inAlbumView && index < albumCount) {
            selectAlbum(index);
        } else if (!inAlbumView && index < trackCount) {
            selectTrack(index);
        }
        return;
    }
    
    // Scroll arrows
    if (x > LIST_X + LIST_W) {
        if (y < LIST_Y + 40 && scrollOffset > 0) {
            scrollList(-1);  // Scroll up
        } else if (y > LIST_Y + LIST_H - 40) {
            scrollList(1);  // Scroll down
        }
    }
}

void MP3Screen::playTrack(int index) {
    if (index < 0 || index >= trackCount) return;
    
    selectedTrack = index;
    
    // Build full path: /Music/AlbumName/TrackName.mp3
    char trackPath[256];
    snprintf(trackPath, sizeof(trackPath), "/Music/%s/%s",  // ADD /Music/
             albumNames[selectedAlbum], trackNames[index]);
    
    Serial.printf("Playing: %s\n", trackPath);
    
    extern MP3Player mp3Player;
    mp3Player.play(trackPath);
    
    isPlaying = true;
    playPauseButton.setLabel("Pause");
    playPauseButton.draw(tft);
}


void MP3Screen::nextTrack() {
    selectedTrack++;
    if (selectedTrack >= trackCount) {
        selectedTrack = 0;  // Loop album
    }
    playTrack(selectedTrack);
    drawLayout();  // Update selection highlight
}