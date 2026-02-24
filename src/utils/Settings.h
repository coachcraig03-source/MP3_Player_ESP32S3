// =====================================================================
//  Settings.h - Configuration file manager
// =====================================================================

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

class Settings {
public:
    static Settings& getInstance();
    
    bool load();  // Load from /Settings/config.txt
    bool save();  // Save to /Settings/config.txt
    
    // WiFi settings
    String wifi_ssid;
    String wifi_password;
    int wifi_timeout;
    
    // NTP settings
    long ntp_offset;
    int ntp_daylight;
    
    // Volume
    int default_volume;
    
private:
    Settings();
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    
    String readLine(class FsFile& file);
    void parseLine(const String& line);
};

#endif // SETTINGS_H
