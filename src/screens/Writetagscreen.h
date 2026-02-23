// =====================================================================
//  WriteTagScreen.h - NFC Tag Writing Screen
// =====================================================================

#ifndef WRITE_TAG_SCREEN_H
#define WRITE_TAG_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"
#include "../utils/SD_Module.h"
#include "../utils/RC522_Module.h"

class ScreenManager;
class TFT_Module;

class WriteTagScreen : public BaseScreen {
public:
    WriteTagScreen(ScreenManager& manager, TFT_Module& tft, SD_Module& sd, RC522_Module& nfc);
    
    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    void loadAlbumsFromSD();
    void drawAlbumList();
    void selectAlbum(int index);
    void waitForTag();
    void writeTag();
    
    SD_Module& sdModule;
    RC522_Module& nfcModule;
    
    UIButton backButton;
    
    char albumNames[50][64];
    int albumCount;
    int selectedAlbum;
    int scrollOffset;
    
    enum State {
        SELECTING_ALBUM,
        WAITING_FOR_TAG,
        WRITING,
        SUCCESS,
        ERROR
    };
    State currentState;
};

#endif // WRITE_TAG_SCREEN_H