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
    
    // Read chunk of data (returns bytes read, 0 = EOF)
    size_t readChunk(uint8_t* buffer, size_t size);
    
    // Check if current file is still open
    bool isFileOpen() const { return currentFile.isOpen(); }
    
private:
    uint8_t _cs;
    bool initialized;
    
    SdFat sd;
    FsFile currentFile;
};

#endif // SD_MODULE_H
