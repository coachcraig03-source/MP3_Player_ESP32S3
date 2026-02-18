// =====================================================================
//  AdultScreen.h - Traditional MP3 player with album list
// =====================================================================

#ifndef ADULT_SCREEN_H
#define ADULT_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

#define MAX_VISIBLE_ALBUMS 6

class AdultScreen : public BaseScreen {
public:
    AdultScreen(ScreenManager& manager, TFT_Module& tft);

    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;
    
    // Called to populate album list (from SD card scan)
    void setAlbumList(const char** albums, int count);
    
    // Select and play an album
    void playAlbum(int index);

private:
    void drawAlbumList();
    void drawPlaybackInfo();
    int getTouchedAlbumIndex(int y);
    
    const char** albumList;
    int albumCount;
    int scrollOffset;
    int selectedAlbum;
    bool isPlaying;
    
    UIButton backButton;
    UIButton playPauseButton;
    UIButton prevButton;
    UIButton nextButton;
};

#endif // ADULT_SCREEN_H
