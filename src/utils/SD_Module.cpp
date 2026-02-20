// =====================================================================
//  SD_Module.cpp - SD Card Implementation
// =====================================================================

#include "SD_Module.h"
#include <SPI.h>

#define SPI1_SCK  12
#define SPI1_MISO 13
#define SPI1_MOSI 11

SD_Module::SD_Module(uint8_t cs)
    : _cs(cs), initialized(false)
{
}

bool SD_Module::begin() {
    Serial.println("SD: Initializing...");
    
    // Initialize SPI1 bus
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(10);
    
    if (!sd.begin(_cs, SD_SCK_MHZ(25))) {
        Serial.println("SD: ✗ Initialization failed!");
        return false;
    }
    
    initialized = true;
    
    Serial.println("SD: ✓ Card initialized");
    Serial.printf("SD: Card size: %.2f MB\n", 
                  sd.card()->sectorCount() * 512.0 / 1048576.0);
    
    return true;
}

bool SD_Module::getFirstMP3(char* path, size_t pathSize) {
    if (!initialized) {
        Serial.println("SD: Not initialized!");
        return false;
    }
    
    Serial.println("SD: Searching for first MP3...");
    
    // Open root directory
    FsFile root;
    if (!root.open("/")) {
        Serial.println("SD: Failed to open root");
        return false;
    }
    
    // Search through directories for first MP3
    FsFile dir;
    while (dir.openNext(&root, O_RDONLY)) {
        if (dir.isDirectory()) {
            char dirName[64];
            dir.getName(dirName, sizeof(dirName));
            
            // Look for MP3 in this directory
            FsFile file;
            while (file.openNext(&dir, O_RDONLY)) {
                char fileName[64];
                file.getName(fileName, sizeof(fileName));
                
                if (strstr(fileName, ".mp3") || strstr(fileName, ".MP3")) {
                    // Found an MP3! Build full path
                    snprintf(path, pathSize, "/%s/%s", dirName, fileName);
                    Serial.printf("SD: Found MP3: %s\n", path);
                    
                    file.close();
                    dir.close();
                    root.close();
                    return true;
                }
                file.close();
            }
        }
        dir.close();
    }
    
    root.close();
    Serial.println("SD: No MP3 files found");
    return false;
}

bool SD_Module::openFile(const char* path) {
    if (!initialized) {
        Serial.println("SD: Not initialized!");
        return false;
    }
    
    // Close any previously open file
    if (currentFile.isOpen()) {
        currentFile.close();
    }
    
    // Reinitialize SPI1 before opening file
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(5);
    
    if (!currentFile.open(path, O_RDONLY)) {
        Serial.printf("SD: Failed to open %s\n", path);
        return false;
    }
    
Serial.printf("SD: Opened %s (raw size=%lu, cast size=%lu)\n", 
              path, 
              (unsigned long)currentFile.fileSize(),
              (unsigned long)currentFile.size());
    return true;
}

void SD_Module::closeFile() {
    if (currentFile.isOpen()) {
        currentFile.close();
        Serial.println("SD: File closed");
    }
}

size_t SD_Module::readChunk(uint8_t* buffer, size_t size) {
    if (!currentFile.isOpen()) {
        return 0;
    }
    
    // Reinitialize SPI1 before reading
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(1);
    
    return currentFile.read(buffer, size);
}

// Add this method to SD_Module.cpp

bool SD_Module::getAlbumArt(const char* folderPath, char* artPath, size_t pathSize) {
    if (!initialized) {
        Serial.println("SD: Not initialized!");
        return false;
    }
    
    // Reinitialize SPI1
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(5);
    
    // Open the folder
    FsFile dir;
    if (!dir.open(folderPath)) {
        Serial.printf("SD: Failed to open folder %s\n", folderPath);
        return false;
    }
    
    // Look for common album art filenames
    const char* artNames[] = {"folder.jpg", "cover.jpg", "album.jpg", "front.jpg"};
    
    for (int i = 0; i < 4; i++) {
        FsFile artFile;
        if (artFile.open(&dir, artNames[i], O_RDONLY)) {
            // Found album art!
            snprintf(artPath, pathSize, "%s/%s", folderPath, artNames[i]);
            Serial.printf("SD: Found album art: %s\n", artPath);
            artFile.close();
            dir.close();
            return true;
        }
    }
    
    dir.close();
    Serial.printf("SD: No album art found in %s\n", folderPath);
    return false;
}