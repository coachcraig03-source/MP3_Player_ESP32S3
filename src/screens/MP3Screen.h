// =====================================================================
//  MP3Screen.h - MP3 Library Browser Screen
// =====================================================================

#ifndef MP3_SCREEN_H
#define MP3_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"
#include "../ui/UISlider.h"

class TFT_Module;
class SD_Module;
class VS1053_Module;

class MP3Screen : public BaseScreen {
public:
    MP3Screen(ScreenManager& manager, TFT_Module& tft, SD_Module& sd, VS1053_Module& audio);
    
    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;
    void nextTrack();  

    
private:
    void drawLayout();
    void drawAlbumList();
    void drawTrackList();
    void drawAlbumArt();
    void loadAlbumsFromSD();
    void selectAlbum(int index);
    void selectTrack(int index);
    void scrollList(int direction);  // +1 or -1
    void playTrack(int index);
    
    SD_Module& sdModule;
    VS1053_Module& audioModule;
    
    // Album/track data
    char albumNames[50][64];  // Max 50 albums
    int albumCount;
    int selectedAlbum;
    
    char trackNames[100][64];  // Max 100 tracks per album
    int trackCount;
    int selectedTrack;
    
    bool inAlbumView;  // true = album list, false = track list
    int scrollOffset;
    
    // UI components
    UIButton backButton;
    UIButton prevButton;
    UIButton playPauseButton;
    UIButton nextButton;
    UISlider volumeSlider;
    
    bool isPlaying;
};

#endif // MP3_SCREEN_H