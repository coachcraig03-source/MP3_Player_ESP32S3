// =====================================================================
//  MP3AlbumList.cpp - Paginated album list screen
//  Title font: default (Font0), size 3
//  Row font:   FreeSerif9pt7b
// =====================================================================

#include "MP3AlbumList.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../utils/SD_Module.h"
#include "../utils/SPIBusLock.h"
#include <LovyanGFX.hpp>
#include <lgfx/v1/lgfx_fonts.hpp>
#include <SdFat.h>

#define ROW_MARGIN_X  20
#define ROW_WIDTH     440
#define ROW_START_Y   50
#define ROW_HEIGHT    29

MP3AlbumList::MP3AlbumList(ScreenManager& manager, TFT_Module& tftModule, SD_Module& sd)
    : BaseScreen(manager, tftModule),
      sdModule(sd),
      albumCount(0),
      currentPage(0),
      totalPages(1),
      backButton(10, 10, 80, 40, "Back"),
      prevPageButton(60, 275, 150, 40, "< Prev"),
      nextPageButton(270, 275, 150, 40, "Next >")
{
    backButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    prevPageButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    nextPageButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
}

void MP3AlbumList::loadAlbumsFromSD() {
    // Direct access to the global `sd` object, not through SD_Module's
    // own locked methods - same pattern as the old MP3Screen and
    // KidScreen code. This MUST be guarded, since it was the actual root
    // cause of the reproducible SPI bus crash found and fixed earlier
    // this session (Core 1 SD access colliding with Core 0 VS1053
    // streaming with no lock protecting this specific access path).
    SPIBusGuard guard;

    Serial.println("MP3AlbumList: Loading albums from SD...");

    if (!sdModule.isInitialized()) {
        Serial.println("MP3AlbumList: SD not initialized");
        return;
    }

    extern SdFs sd;
    FsFile root;
    if (!root.open("/Music")) {
        Serial.println("MP3AlbumList: Failed to open /Music");
        return;
    }

    albumCount = 0;
    FsFile dir;

    while (dir.openNext(&root, O_RDONLY) && albumCount < MAX_ALBUMS) {
        if (dir.isDirectory()) {
            char name[64];
            dir.getName(name, sizeof(name));

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
    Serial.printf("MP3AlbumList: Loaded %d albums\n", albumCount);

    totalPages = (albumCount + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
    if (totalPages < 1) totalPages = 1;
}

void MP3AlbumList::begin() {
    currentPage = 0;

    // Only scan the card once - same pattern as the old MP3Screen, so
    // returning to this screen later doesn't re-scan every time.
    if (albumCount == 0) {
        loadAlbumsFromSD();
    }

    drawPage();
}

void MP3AlbumList::drawPage() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);

    // Title
    display->setFont(&fonts::Font0);
    display->setTextSize(3);
    display->setTextColor(TFT_CYAN);
    display->setTextDatum(top_center);
    display->drawString("Albums", 240, 8);

    // Page indicator
    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->setTextDatum(top_right);
    char pageStr[24];
    snprintf(pageStr, sizeof(pageStr), "Page %d of %d", currentPage + 1, totalPages);
    display->drawString(pageStr, 465, 20);

    backButton.draw(tft);

    if (albumCount == 0) {
        display->setFont(&fonts::Font0);
        display->setTextSize(2);
        display->setTextColor(TFT_DARKGREY);
        display->setTextDatum(top_left);
        display->drawString("No albums found on SD card", ROW_MARGIN_X, ROW_START_Y);
    } else {
        int startIndex = currentPage * ROWS_PER_PAGE;
        for (int i = 0; i < ROWS_PER_PAGE; i++) {
            int albumIndex = startIndex + i;
            if (albumIndex >= albumCount) break;

            int rowY = ROW_START_Y + (i * ROW_HEIGHT);

            display->setFont(&fonts::FreeSerif9pt7b);
            display->setTextSize(1, 1);
            display->setTextColor(TFT_WHITE);
            display->setTextDatum(middle_left);

            String name = String(albumNames[albumIndex]);
            if (name.length() > 55) {
                name = name.substring(0, 52) + "...";
            }
            display->drawString(name, ROW_MARGIN_X + 12, rowY + ROW_HEIGHT / 2);
        }
    }

    prevPageButton.draw(tft);
    nextPageButton.draw(tft);

    // Reset to the default font before leaving this function - without
    // this, whatever screen comes next inherits FreeSerif9pt7b as the
    // active font, since setFont() state persists across screens.
    display->setFont(&fonts::Font0);
}

void MP3AlbumList::nextPage() {
    if (currentPage < totalPages - 1) {
        currentPage++;
        drawPage();
    }
}

void MP3AlbumList::prevPage() {
    if (currentPage > 0) {
        currentPage--;
        drawPage();
    }
}

void MP3AlbumList::update() {
    // Nothing animated yet.
}

void MP3AlbumList::handleTouch(int x, int y) {
    if (backButton.hit(x, y)) {
        Serial.println("MP3AlbumList: Back pressed");
        screenManager.showSplash();
        return;
    }

    if (prevPageButton.hit(x, y)) {
        Serial.println("MP3AlbumList: Prev page");
        prevPage();
        return;
    }

    if (nextPageButton.hit(x, y)) {
        Serial.println("MP3AlbumList: Next page");
        nextPage();
        return;
    }

    // Album row selection - figure out exactly which album was tapped
    // and hand it off to the song list screen before navigating.
    if (x >= ROW_MARGIN_X && x < ROW_MARGIN_X + ROW_WIDTH &&
        y >= ROW_START_Y && y < ROW_START_Y + (ROWS_PER_PAGE * ROW_HEIGHT)) {
        int rowIndex = (y - ROW_START_Y) / ROW_HEIGHT;
        int albumIndex = (currentPage * ROWS_PER_PAGE) + rowIndex;

        if (rowIndex < ROWS_PER_PAGE && albumIndex < albumCount) {
            Serial.printf("MP3AlbumList: Selected '%s'\n", albumNames[albumIndex]);
            screenManager.getSongListScreen()->loadAlbum(albumNames[albumIndex]);
            screenManager.showSongList();
        }
        return;
    }
}
