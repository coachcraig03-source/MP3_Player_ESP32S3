// =====================================================================
//  MP3AlbumList.h - Paginated album list screen
// =====================================================================

#ifndef MP3_ALBUM_LIST_H
#define MP3_ALBUM_LIST_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

class ScreenManager;
class TFT_Module;
class SD_Module;

class MP3AlbumList : public BaseScreen {
public:
    MP3AlbumList(ScreenManager& manager, TFT_Module& tft, SD_Module& sd);

    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    static const int ROWS_PER_PAGE = 7;
    static const int MAX_ALBUMS = 50;

    void drawPage();
    void nextPage();
    void prevPage();
    void loadAlbumsFromSD();

    SD_Module& sdModule;

    char albumNames[MAX_ALBUMS][64];
    int albumCount;

    int currentPage;
    int totalPages;

    UIButton backButton;
    UIButton prevPageButton;
    UIButton nextPageButton;
};

#endif // MP3_ALBUM_LIST_H