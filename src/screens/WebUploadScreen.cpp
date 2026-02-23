// =====================================================================
//  WebUploadScreen.cpp - Simple WiFiServer File Browser
// =====================================================================

#include "WebUploadScreen.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include <LovyanGFX.hpp>
#include <SdFat.h>

WebUploadScreen::WebUploadScreen(ScreenManager& manager, TFT_Module& tftModule, SD_Module& sd)
    : BaseScreen(manager, tftModule),
      sdModule(sd),
      webServer(nullptr),
      doneButton(160, 260, 160, 50, "Done"),
      serverActive(false),
      lastUpdate(0),
      uploadCount(0)
{
    doneButton.setColors(TFT_RED, TFT_WHITE, TFT_WHITE);
}

void WebUploadScreen::begin() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);
    
    // Title
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->setTextSize(3);
    display->drawString("Web Upload", 240, 20);
    
    // Check WiFi status
    if (WiFi.status() != WL_CONNECTED) {
        display->setTextSize(2);
        display->setTextColor(TFT_RED);
        display->drawString("WiFi Not Connected!", 240, 120);
        display->setTextSize(1);
        display->setTextColor(TFT_WHITE);
        display->drawString("Please restart device", 240, 160);
        
        doneButton.draw(tft);
        return;
    }
    
    // Show connection instructions
    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->drawString("1. Connect to WiFi:", 240, 80);
    display->setTextSize(2);
    display->setTextColor(TFT_YELLOW);
    display->drawString("MP3Player", 240, 105);

    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->drawString("2. Password:", 240, 135);
    display->setTextSize(2);
    display->setTextColor(TFT_YELLOW);
    display->drawString("12345678", 240, 160);

    display->setTextSize(1);
    display->setTextColor(TFT_CYAN);
    display->drawString("3. Open browser:", 240, 190);
    display->setTextSize(2);
    display->setTextColor(TFT_YELLOW);
    display->drawString("http://192.168.4.1", 240, 215);
    
    doneButton.draw(tft);
    
    // Start web server
    startWebServer();
}

void WebUploadScreen::startWebServer() {
    Serial.println("Starting WiFi AP and web server...");
    
    // Stop MP3 task
    //extern TaskHandle_t mp3TaskHandle;
    //if (mp3TaskHandle) {
     //   vTaskDelete(mp3TaskHandle);
     //   mp3TaskHandle = nullptr;
   // }
    
    delay(500);
    
    // Switch to AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP("MP3Player", "12345678");
    
    delay(1000);
    
    IPAddress IP = WiFi.softAPIP();
    Serial.printf("AP Started!\n");
    Serial.printf("SSID: MP3Player\n");
    Serial.printf("Password: 12345678\n");
    Serial.printf("IP: %s\n", IP.toString().c_str());
    
    webServer = new WiFiServer(80);
    webServer->begin();
    serverActive = true;
    
    Serial.println("WiFi server started");
}

void WebUploadScreen::stopWebServer() {
    if (webServer) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
    }
    serverActive = false;
    Serial.println("Web server stopped");
}

void WebUploadScreen::update() {
    if (serverActive && webServer) {
        handleClient();
        
        // Update status display every 2 seconds
        if (millis() - lastUpdate > 2000) {
            lastUpdate = millis();
            updateStatus();
        }
    }
}

void WebUploadScreen::handleClient() {
    WiFiClient client = webServer->available();
    if (!client) return;
    
    Serial.println("New client connected");
        Serial.printf("Client IP: %s\n", client.remoteIP().toString().c_str());
    
    // Wait for data
    unsigned long timeout = millis() + 5000;
    while (!client.available() && millis() < timeout) {
        delay(1);
    }
    
    if (!client.available()) {
        Serial.println("Client timeout - no data received");
        client.stop();
        return;
    }
    
    // Read request
    String request = client.readStringUntil('\r');
    client.flush();
    
    Serial.println("Request: " + request);
    
    // Parse request
    if (request.indexOf("GET /") >= 0) {
        // Send main page
        sendFileBrowser(client);
    } 
    else if (request.indexOf("GET /list") >= 0) {
        // Send file list JSON
        sendFileList(client);
    }
    else if (request.indexOf("POST /upload") >= 0) {
        // Handle file upload
        handleUpload(client);
    }
    else {
        // 404
        client.println("HTTP/1.1 404 Not Found");
        client.println();
    }
    
    delay(10);
    client.stop();
}

void WebUploadScreen::sendFileBrowser(WiFiClient& client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    
    // Simple HTML page
    client.println("<!DOCTYPE html><html><head><title>MP3 Upload</title>");
    client.println("<style>body{font-family:Arial;margin:20px;background:#f0f0f0}");
    client.println("h1{color:#333}.file{padding:8px;margin:3px;background:white}");
    client.println(".upload{background:#4CAF50;color:white;padding:10px 20px;border:none;cursor:pointer}");
    client.println("</style></head><body>");
    client.println("<h1>🎵 MP3 Player Upload</h1>");
    client.println("<form method='POST' action='/upload' enctype='multipart/form-data'>");
    client.println("<input type='file' name='file' accept='.mp3,.wma'>");
    client.println("<button class='upload'>Upload to /Music</button></form><hr>");
    client.println("<h2>Files in /Music:</h2><div id='files'>");
    
    // List files
    extern SdFat sd;
    FsFile root;
    if (root.open("/Music")) {
        FsFile file;
        while (file.openNext(&root, O_RDONLY)) {
            char name[128];
            file.getName(name, sizeof(name));
            
            client.print("<div class='file'>");
            if (file.isDirectory()) {
                client.print("📁 <b>");
                client.print(name);
                client.print("</b>");
            } else {
                client.print("📄 ");
                client.print(name);
            }
            client.println("</div>");
            file.close();
        }
        root.close();
    }
    
    client.println("</div></body></html>");
}

void WebUploadScreen::sendFileList(WiFiClient& client) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println();
    client.println("{\"files\":[]}");  // Simple for now
}

void WebUploadScreen::handleUpload(WiFiClient& client) {
    Serial.println("Handling upload...");
    
    // Read headers to find content boundary
    String boundary = "";
    String line;
    
    while (client.connected()) {
        line = client.readStringUntil('\n');
        if (line.indexOf("boundary=") >= 0) {
            int idx = line.indexOf("boundary=");
            boundary = "--" + line.substring(idx + 9);
            boundary.trim();
            Serial.println("Boundary: " + boundary);
        }
        if (line == "\r") break;  // End of headers
    }
    
    if (boundary.length() == 0) {
        client.println("HTTP/1.1 400 Bad Request");
        client.println();
        return;
    }
    
    // Find filename
    String filename = "";
    while (client.connected()) {
        line = client.readStringUntil('\n');
        if (line.indexOf("filename=") >= 0) {
            int start = line.indexOf("filename=\"") + 10;
            int end = line.indexOf("\"", start);
            filename = line.substring(start, end);
            Serial.println("Filename: " + filename);
        }
        if (line == "\r") break;  // Start of file data
    }
    
    if (filename.length() == 0) {
        client.println("HTTP/1.1 400 Bad Request");
        client.println();
        return;
    }
    
    // Save file
    String filepath = "/Music/" + filename;
    extern SdFat sd;
    FsFile uploadFile;
    
    if (uploadFile.open(filepath.c_str(), O_WRITE | O_CREAT | O_TRUNC)) {
        Serial.println("File opened for writing: " + filepath);
        
        // Read and save file data
        uint8_t buffer[512];
        int bytesRead = 0;
        
        while (client.connected()) {
            if (client.available()) {
                buffer[bytesRead++] = client.read();
                
                if (bytesRead >= 512) {
                    uploadFile.write(buffer, bytesRead);
                    bytesRead = 0;
                }
                
                // Check for boundary (end of file)
                if (bytesRead > boundary.length()) {
                    String check = String((char*)buffer);
                    if (check.indexOf(boundary) >= 0) {
                        // Write remaining and finish
                        uploadFile.write(buffer, bytesRead - boundary.length() - 4);
                        break;
                    }
                }
            }
        }
        
        uploadFile.close();
        uploadCount++;
        
        Serial.println("Upload complete!");
        
        // Send success response
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<html><body><h1>Upload Success!</h1>");
        client.println("<p>File saved to: " + filepath + "</p>");
        client.println("<a href='/'>Back</a></body></html>");
    } else {
        Serial.println("Failed to open file");
        client.println("HTTP/1.1 500 Internal Server Error");
        client.println();
    }
}

void WebUploadScreen::updateStatus() {
    auto display = tft.getTFT();

    // Update upload count
    display->fillRect(180, 230, 120, 20, TFT_BLACK);
    display->setTextSize(1);
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(middle_center);
    char countStr[32];
    snprintf(countStr, sizeof(countStr), "Files uploaded: %d", uploadCount);
    display->drawString(countStr, 240, 230);
    
    // Show active indicator
    static bool blink = false;
    blink = !blink;
    display->fillCircle(450, 30, 10, blink ? TFT_GREEN : TFT_DARKGREEN);
}

void WebUploadScreen::handleTouch(int x, int y) {
    if (doneButton.hit(x, y)) {
        Serial.println("Exiting upload mode");
        stopWebServer();
        screenManager.showSettings();
        return;
    }
}

String WebUploadScreen::getContentType(String filename) {
    if (filename.endsWith(".html")) return "text/html";
    else if (filename.endsWith(".css")) return "text/css";
    else if (filename.endsWith(".js")) return "application/javascript";
    else if (filename.endsWith(".json")) return "application/json";
    else if (filename.endsWith(".mp3")) return "audio/mpeg";
    return "application/octet-stream";
}