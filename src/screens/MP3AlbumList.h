// =====================================================================
//  MP3AlbumList.h - Paginated album list screen (UI-only prototype)
//
//  Purpose: get the layout, font size, row height, and pagination feel
//  right on real hardware before wiring in real album data. Only the
//  Back button and page navigation are functional; album rows are not
//  tappable yet (no selection logic - that comes once the UI is settled
//  and this replaces the album-browsing half of MP3Screen).
// =====================================================================

#ifndef MP3_ALBUM_LIST_H
#define MP3_ALBUM_LIST_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

class ScreenManager;
class TFT_Module;

class MP3AlbumList : public BaseScreen {
public:
    MP3AlbumList(ScreenManager& manager, TFT_Module& tft);

    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    static const int ROWS_PER_PAGE = 7;
    static const int PLACEHOLDER_COUNT = 13;  // enough to span 3 pages for testing

    void drawPage();
    void nextPage();
    void prevPage();

    const char* placeholderAlbums[PLACEHOLDER_COUNT] = {
        "Abba - Gold (Greatest Hits)",
        "Bob Seger - Against the Wind",
        "Bob Seger - Greatest Hits",
        "Bob Seger - Night Moves",
        "Bon Jovi - The Best of - Cross Road",
        "Boston - Greatest Hits",
        "Bruce Springsteen - Born To Run",
        "Bruce Springsteen - The River",
        "Carol King - Tapestry",
        "Cher - Heart of Stone",
        "Doobie Brothers - Listen to The Music",
        "Eagles - Hotel California",
        "Supertramp - Crime Of The Century"
    };

    int currentPage;
    int totalPages;

    UIButton backButton;
    UIButton prevPageButton;
    UIButton nextPageButton;
};

#endif // MP3_ALBUM_LIST_H
