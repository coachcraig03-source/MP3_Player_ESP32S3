// =====================================================================
//  FTPUploadScreen.h - FTP Server in AP Mode
// =====================================================================

#ifndef FTP_UPLOAD_SCREEN_H
#define FTP_UPLOAD_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"
#include "../utils/SD_Module.h"
#include <WiFi.h>
#include "ESP32FtpServer.h"


class ScreenManager;
class TFT_Module;

class FTPUploadScreen : public BaseScreen {
public:
    FTPUploadScreen(ScreenManager& manager, TFT_Module& tft, SD_Module& sd);
    
    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:

    void startFTPServer();
    void stopFTPServer();
    void updateStatus();
    
    SD_Module& sdModule;
    FtpServer* ftpServer;  // Note: might be FtpServer not FTPServer
    
    UIButton doneButton;
    
    bool serverActive;
    unsigned long lastUpdate;
};

#endif // FTP_UPLOAD_SCREEN_H