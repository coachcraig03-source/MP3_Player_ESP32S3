// =====================================================================
//  WriteTagScreen.cpp - NFC Tag Writing Implementation
// =====================================================================

#include "WriteTagScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>
#include <SdFat.h>

#define LIST_X 40
#define LIST_Y 80
#define LIST_W 360 
#define LIST_H 200
#define ITEM_HEIGHT 25

WriteTagScreen::WriteTagScreen(ScreenManager& manager, TFT_Module& tftModule, SD_Module& sd, RC522_Module& nfc)
    : BaseScreen(manager, tftModule),
      sdModule(sd),
      nfcModule(nfc),
      backButton(10, 10, 80, 40, "Back"),
      albumCount(0),
      selectedAlbum(-1),
      scrollOffset(0),
      currentState(SELECTING_ALBUM)
{
    backButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
}

void WriteTagScreen::begin() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);
    
    // Title
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->setTextSize(2);
    display->drawString("Write NFC Tag", 240, 15);
    
    backButton.draw(tft);
    
    // Load albums from SD
    loadAlbumsFromSD();
    
    currentState = SELECTING_ALBUM;
    drawAlbumList();
}

void WriteTagScreen::loadAlbumsFromSD() {
    albumCount = 0;
    
    extern SdFat sd;
    FsFile root;
    if (!root.open("/Music")) {
        Serial.println("WriteTag: Failed to open /Music");
        return;
    }
    
    FsFile dir;
    while (dir.openNext(&root, O_RDONLY) && albumCount < 50) {
        if (dir.isDirectory()) {
            char name[64];
            dir.getName(name, sizeof(name));
            
            // Skip system directories
            if (name[0] == '.' || strcmp(name, "System Volume Information") == 0) {
                dir.close();
                continue;
            }
            
            strncpy(albumNames[albumCount], name, sizeof(albumNames[0]) - 1);
            albumNames[albumCount][sizeof(albumNames[0]) - 1] = '\0';
            albumCount++;
        }
        dir.close();
    }
    root.close();
    
    Serial.printf("WriteTag: Loaded %d albums\n", albumCount);
}

void WriteTagScreen::drawAlbumList() {
    auto display = tft.getTFT();
    
    // Clear list area
    display->fillRect(LIST_X, LIST_Y, LIST_W, LIST_H, TFT_BLACK);
    display->drawRect(LIST_X, LIST_Y, LIST_W, LIST_H, TFT_WHITE);
    
    // Instructions
    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->setTextDatum(top_left);
    display->drawString("Select album to write to NFC tag:", LIST_X + 10, LIST_Y - 20);
    
    // Draw visible albums
    int maxVisible = LIST_H / ITEM_HEIGHT;
    display->setTextSize(1);
    
    for (int i = 0; i < maxVisible && (scrollOffset + i) < albumCount; i++) {
        int index = scrollOffset + i;
        int yPos = LIST_Y + 5 + (i * ITEM_HEIGHT);
        
        // Highlight selected
        if (index == selectedAlbum) {
            display->fillRect(LIST_X + 2, yPos - 2, LIST_W - 4, ITEM_HEIGHT, TFT_BLUE);
        }
        
        // Draw album name
        display->setTextColor(TFT_WHITE);
        display->setTextDatum(top_left);
        display->drawString(albumNames[index], LIST_X + 10, yPos);
    }
    
    // Draw scroll arrows (right side with spacing)
    int arrowX = LIST_X + LIST_W + 20;  // 20px spacing
    
    // Up arrow
    if (scrollOffset > 0) {
        display->fillTriangle(arrowX, LIST_Y + 25, 
                             arrowX - 10, LIST_Y + 35,
                             arrowX + 10, LIST_Y + 35, TFT_CYAN);
    }
    
    // Down arrow
    if (scrollOffset < albumCount - maxVisible) {
        display->fillTriangle(arrowX, LIST_Y + LIST_H - 25,
                             arrowX - 10, LIST_Y + LIST_H - 35,
                             arrowX + 10, LIST_Y + LIST_H - 35, TFT_CYAN);
    }
}

void WriteTagScreen::selectAlbum(int index) {
    if (index < 0 || index >= albumCount) return;
    
    selectedAlbum = index;
    Serial.printf("WriteTag: Selected '%s'\n", albumNames[index]);
    
    currentState = WAITING_FOR_TAG;
    
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);
    
    // Title
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->setTextSize(2);
    display->drawString("Write NFC Tag", 240, 15);
    
    backButton.draw(tft);
    
    // Instructions
    display->setTextSize(2);
    display->setTextColor(TFT_CYAN);
    display->setTextDatum(middle_center);
    display->drawString("Place blank NFC tag", 240, 120);
    display->drawString("on reader...", 240, 150);
    
    // Show album name
    display->setTextSize(1);
    display->setTextColor(TFT_YELLOW);
    display->drawString("Writing:", 240, 200);
    display->setTextSize(2);
    display->drawString(albumNames[selectedAlbum], 240, 220);
}

void WriteTagScreen::update() {
    if (currentState == WAITING_FOR_TAG) {
        // Check for tag presence
        if (nfcModule.isCardPresent()) {
            writeTag();
        }
    }
}

void WriteTagScreen::writeTag() {
    auto display = tft.getTFT();
    
    currentState = WRITING;
    display->fillRect(0, 100, 480, 150, TFT_BLACK);
    display->setTextSize(2);
    display->setTextColor(TFT_YELLOW);
    display->setTextDatum(middle_center);
    display->drawString("Writing tag...", 240, 150);
    
    // Write the tag
    if (nfcModule.writeAlbumTag(albumNames[selectedAlbum])) {
        currentState = SUCCESS;
        display->fillRect(0, 100, 480, 150, TFT_BLACK);
        display->setTextColor(TFT_GREEN);
        display->drawString("Success!", 240, 120);
        display->setTextSize(1);
        display->setTextColor(TFT_WHITE);
        display->drawString("Tag written for:", 240, 160);
        display->setTextSize(2);
        display->drawString(albumNames[selectedAlbum], 240, 180);
        display->setTextSize(1);
        display->setTextColor(TFT_CYAN);
        display->drawString("Remove tag and press Back", 240, 250);
        
        Serial.printf("WriteTag: Success - '%s'\n", albumNames[selectedAlbum]);
    } else {
        currentState = ERROR;
        display->fillRect(0, 100, 480, 150, TFT_BLACK);
        display->setTextColor(TFT_RED);
        display->drawString("Write Failed!", 240, 150);
        display->setTextSize(1);
        display->setTextColor(TFT_WHITE);
        display->drawString("Check tag and try again", 240, 180);
        
        Serial.println("WriteTag: Write failed");
        delay(2000);
        currentState = SELECTING_ALBUM;
        begin();
    }
}

void WriteTagScreen::handleTouch(int x, int y) {
    if (backButton.hit(x, y)) {
        screenManager.showSettings();
        return;
    }
    
    if (currentState == SELECTING_ALBUM) {
        // List area
        if (x >= LIST_X && x < LIST_X + LIST_W && y >= LIST_Y && y < LIST_Y + LIST_H) {
            int relativeY = y - LIST_Y - 5;
            int index = scrollOffset + (relativeY / ITEM_HEIGHT);
            
            if (index < albumCount) {
                selectAlbum(index);
            }
            return;
        }
        
        // Scroll arrows (separate from list)
        int arrowX = LIST_X + LIST_W + 20;
        if (x > arrowX - 15 && x < arrowX + 15) {
            if (y >= LIST_Y && y < LIST_Y + 50 && scrollOffset > 0) {
                scrollOffset--;
                drawAlbumList();
            } else if (y > LIST_Y + LIST_H - 50 && y <= LIST_Y + LIST_H && scrollOffset < albumCount - 8) {
                scrollOffset++;
                drawAlbumList();
            }
        }
    }
}