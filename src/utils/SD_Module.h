// =====================================================================
//  SD_Module.h - SD Card Management
// =====================================================================

#ifndef SD_MODULE_H
#define SD_MODULE_H

#include <Arduino.h>
#include <SdFat.h>

class SD_Module {
public:
    SD_Module(uint8_t cs);
    
    bool begin();
    bool isInitialized() const { return initialized; }
    
    // Get first MP3 file on card (for testing)
    bool getFirstMP3(char* path, size_t pathSize);
    
    // Open a file for reading
    bool openFile(const char* path);
    void closeFile();
    
    // Get album art path for a folder
    bool getAlbumArt(const char* folderPath, char* artPath, size_t pathSize);
    
    // Read chunk of data (returns bytes read, 0 = EOF)
    size_t readChunk(uint8_t* buffer, size_t size);
    
    // Check if current file is still open
    bool isFileOpen() const { return currentFile.isOpen(); }
    
private:
    uint8_t _cs;
    bool initialized;
    
    FsFile currentFile;

    // --- instrumentation only, no behavioral effect ---
    unsigned long _readCount = 0;
    unsigned long _slowReadCount = 0;
    unsigned long _worstReadUs = 0;
};

#endif // SD_MODULE_H