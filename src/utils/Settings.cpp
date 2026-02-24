// =====================================================================
//  Settings.cpp - Configuration file implementation
// =====================================================================

#include "Settings.h"
#include <SdFat.h>

Settings& Settings::getInstance() {
    static Settings instance;
    return instance;
}

Settings::Settings()
    : wifi_ssid(""),
      wifi_password(""),
      wifi_timeout(20),
      ntp_offset(-28800),
      ntp_daylight(3600),
      default_volume(55)
{
}

bool Settings::load() {
    extern SdFat sd;
    
    FsFile configFile;
    if (!configFile.open("/Settings/config.txt", O_RDONLY)) {
        Serial.println("Settings: config.txt not found, using defaults");
        return false;
    }
    
    Serial.println("Settings: Loading config.txt");
    
    while (configFile.available()) {
        String line = readLine(configFile);
        if (line.length() > 0 && line.charAt(0) != '#') {
            parseLine(line);
        }
    }
    
    configFile.close();
    
    Serial.println("Settings loaded:");
    Serial.printf("  WiFi SSID: %s\n", wifi_ssid.c_str());
    Serial.printf("  WiFi Timeout: %d\n", wifi_timeout);
    Serial.printf("  NTP Offset: %ld\n", ntp_offset);
    Serial.printf("  Default Volume: %d\n", default_volume);
    
    return true;
}

bool Settings::save() {
    extern SdFat sd;
    
    // Create Settings directory if needed
    if (!sd.exists("/Settings")) {
        sd.mkdir("/Settings");
    }
    
    FsFile configFile;
    if (!configFile.open("/Settings/config.txt", O_WRITE | O_CREAT | O_TRUNC)) {
        Serial.println("Settings: Failed to create config.txt");
        return false;
    }
    
    // Write settings
    configFile.print("# WiFi Configuration\n");
    configFile.print("wifi_ssid=");
    configFile.println(wifi_ssid);
    configFile.print("wifi_password=");
    configFile.println(wifi_password);
    configFile.print("wifi_timeout=");
    configFile.println(wifi_timeout);
    
    configFile.print("\n# NTP Configuration\n");
    configFile.print("ntp_offset=");
    configFile.println(ntp_offset);
    configFile.print("ntp_daylight=");
    configFile.println(ntp_daylight);
    
    configFile.print("\n# Audio Configuration\n");
    configFile.print("default_volume=");
    configFile.println(default_volume);
    
    configFile.close();
    
    Serial.println("Settings saved to /Settings/config.txt");
    return true;
}

String Settings::readLine(FsFile& file) {
    String line = "";
    char c;
    
    while (file.available()) {
        c = file.read();
        if (c == '\n') break;
        if (c != '\r') line += c;
    }
    
    return line;
}

void Settings::parseLine(const String& line) {
    int separator = line.indexOf('=');
    if (separator == -1) return;
    
    String key = line.substring(0, separator);
    String value = line.substring(separator + 1);
    
    key.trim();
    value.trim();
    
    if (key == "wifi_ssid") {
        wifi_ssid = value;
    } else if (key == "wifi_password") {
        wifi_password = value;
    } else if (key == "wifi_timeout") {
        wifi_timeout = value.toInt();
    } else if (key == "ntp_offset") {
        ntp_offset = value.toInt();
    } else if (key == "ntp_daylight") {
        ntp_daylight = value.toInt();
    } else if (key == "default_volume") {
        default_volume = value.toInt();
    }
}
