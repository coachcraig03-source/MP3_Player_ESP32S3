// =====================================================================
//  SD_Module.cpp - SD Card Implementation
// =====================================================================

#include "SD_Module.h"
#include "SPIBusLock.h"
#include <SPI.h>

extern SdFs sd;

#define SPI1_SCK  12
#define SPI1_MISO 13
#define SPI1_MOSI 11

SD_Module::SD_Module(uint8_t cs)
    : _cs(cs), initialized(false)
{
}

bool SD_Module::begin() {
    SPIBusGuard guard;

    Serial.println("SD: Initializing...");
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    delay(250);  // Let card settle longer
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(100);
    
    if (!sd.begin(SdSpiConfig(_cs, SHARED_SPI, SD_SCK_MHZ(25)))) {
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
    SPIBusGuard guard;

    if (!initialized) {
        Serial.println("SD: Not initialized!");
        return false;
    }
    
    Serial.println("SD: Searching for first MP3...");
    
    FsFile root;
    if (!root.open("/")) {
        Serial.println("SD: Failed to open root");
        return false;
    }
    
    FsFile dir;
    while (dir.openNext(&root, O_RDONLY)) {
        if (dir.isDirectory()) {
            char dirName[64];
            dir.getName(dirName, sizeof(dirName));
            
            FsFile file;
            while (file.openNext(&dir, O_RDONLY)) {
                char fileName[64];
                file.getName(fileName, sizeof(fileName));
                
                if (strstr(fileName, ".mp3") || strstr(fileName, ".MP3")) {
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
    SPIBusGuard guard;

    if (!initialized) {
        Serial.println("SD: Not initialized!");
        return false;
    }
    
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

    // Reset instrumentation counters for the new file
    _readCount = 0;
    _slowReadCount = 0;
    _worstReadUs = 0;

    return true;
}

void SD_Module::closeFile() {
    SPIBusGuard guard;

    if (currentFile.isOpen()) {
        currentFile.close();
        Serial.println("SD: File closed");

        // Summary for this file, so we get one line per track instead of
        // spamming per-chunk - easier to correlate with an audible glitch
        // and to see the trend across an album.
        Serial.printf("SD: [stats] reads=%lu slow(>3ms)=%lu worst=%luus\n",
                      (unsigned long)_readCount,
                      (unsigned long)_slowReadCount,
                      (unsigned long)_worstReadUs);
    }
}

size_t SD_Module::readChunk(uint8_t* buffer, size_t size) {
    SPIBusGuard guard;

    if (!currentFile.isOpen()) {
        return 0;
    }

    unsigned long t0 = micros();
    size_t n = currentFile.read(buffer, size);
    unsigned long dt = micros() - t0;

    _readCount++;
    if (dt > _worstReadUs) _worstReadUs = dt;

    // Flag anything slower than 3ms - at 44.1kHz stereo the VS1053's
    // onboard buffer only holds a few milliseconds of audio, so a read
    // stall in this neighborhood is a real glitch candidate.
    if (dt > 3000) {
        _slowReadCount++;
        Serial.printf("SD: [slow read] %lu bytes took %lu us (read #%lu)\n",
                      (unsigned long)n, dt, (unsigned long)_readCount);
    }

    return n;
}

bool SD_Module::getAlbumArt(const char* folderPath, char* artPath, size_t pathSize) {
    SPIBusGuard guard;

    if (!initialized) {
        Serial.println("SD: Not initialized!");
        return false;
    }
    
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(5);
    
    FsFile dir;
    if (!dir.open(folderPath)) {
        Serial.printf("SD: Failed to open folder %s\n", folderPath);
        return false;
    }
    
    const char* artNames[] = {"folder.jpg", "cover.jpg", "album.jpg", "front.jpg"};
    
    for (int i = 0; i < 4; i++) {
        FsFile artFile;
        if (artFile.open(&dir, artNames[i], O_RDONLY)) {
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
