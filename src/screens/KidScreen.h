// =====================================================================
//  KidScreen.h - Kid-friendly NFC-triggered playback screen
// =====================================================================

#ifndef KID_SCREEN_H
#define KID_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"

class VS1053_Module;

class KidScreen : public BaseScreen {
public:
    KidScreen(ScreenManager& manager, TFT_Module& tft, VS1053_Module& audio);

    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;
    
    // Called when NFC tag is detected
    void showAlbum(const char* albumName);
    
    // Called when NFC tag is removed
    void clearAlbum();

private:
    void drawWaitingScreen();
    void drawPlaybackScreen();
    
    VS1053_Module& audioModule;
    
    char currentAlbum[40];
    bool albumLoaded;
    bool isPlaying;
    
    // Playback control buttons
    UIButton prevButton;
    UIButton playPauseButton;
    UIButton nextButton;
    UIButton backButton;  // Return to splash
};

#endif // KID_SCREEN_H
