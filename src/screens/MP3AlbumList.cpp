// =====================================================================
//  MP3AlbumList.cpp - Paginated album list screen (UI-only prototype)
//  Title font: default (Font0), size 3
//  Row font:   FreeSerif9pt7b - confirmed working (yAdvance=22,
//              measured width for a 32-char sample = 242px)
// =====================================================================

#include "MP3AlbumList.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>
#include <lgfx/v1/lgfx_fonts.hpp>

#define ROW_MARGIN_X  20
#define ROW_WIDTH     440
#define ROW_START_Y   50
#define ROW_HEIGHT    29

MP3AlbumList::MP3AlbumList(ScreenManager& manager, TFT_Module& tftModule)
    : BaseScreen(manager, tftModule),
      currentPage(0),
      backButton(10, 10, 80, 40, "Back"),
      prevPageButton(60, 275, 150, 40, "< Prev"),
      nextPageButton(270, 275, 150, 40, "Next >")
{
    backButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    prevPageButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    nextPageButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);

    totalPages = (PLACEHOLDER_COUNT + ROWS_PER_PAGE - 1) / ROWS_PER_PAGE;
}

void MP3AlbumList::begin() {
    currentPage = 0;
    drawPage();
}

void MP3AlbumList::drawPage() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);

    // Title - plain default font, confirmed working throughout
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

    int startIndex = currentPage * ROWS_PER_PAGE;
    for (int i = 0; i < ROWS_PER_PAGE; i++) {
        int albumIndex = startIndex + i;
        int rowY = ROW_START_Y + (i * ROW_HEIGHT);

        if (albumIndex < PLACEHOLDER_COUNT) {
            // Confirmed-working pattern: setFont, then setTextSize(1,1)
            // immediately after, right before the actual draw call.
            display->setFont(&fonts::FreeSerif9pt7b);
            display->setTextSize(1, 1);
            display->setTextColor(TFT_WHITE);
            display->setTextDatum(middle_left);

            String name = String(placeholderAlbums[albumIndex]);
            if (name.length() > 34) {
                name = name.substring(0, 31) + "...";
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

    // Album rows are not tappable yet - UI-only prototype for now.
}
