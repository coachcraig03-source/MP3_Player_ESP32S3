// =====================================================================
//  WebUploadScreen.h - Web File Browser Upload Mode
// =====================================================================

#ifndef WEB_UPLOAD_SCREEN_H
#define WEB_UPLOAD_SCREEN_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"
#include "../utils/SD_Module.h"
#include <WiFi.h>

class ScreenManager;
class TFT_Module;

class WebUploadScreen : public BaseScreen {
public:
    WebUploadScreen(ScreenManager& manager, TFT_Module& tft, SD_Module& sd);
    
    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

private:
    void startWebServer();
    void stopWebServer();
    void updateStatus();
    void handleClient();
    void sendFileBrowser(WiFiClient& client);  // ADD THIS
    void sendFileList(WiFiClient& client);     // ADD THIS
    void handleUpload(WiFiClient& client);     // ADD THIS
    String getContentType(String filename);
    
    SD_Module& sdModule;
    WiFiServer* webServer;
    
    UIButton doneButton;
    
    bool serverActive;
    unsigned long lastUpdate;
    int uploadCount;
};

#endif // WEB_UPLOAD_SCREEN_H