// =====================================================================
//  VS1053_Module.h - VS1053 Audio Module
// =====================================================================

#ifndef VS1053_MODULE_H
#define VS1053_MODULE_H

#include <Arduino.h>

class VS1053_Module {
public:
    VS1053_Module(uint8_t cs, uint8_t dcs, uint8_t dreq, uint8_t rst);
    
    void begin();
    bool isAlive();
    void getChipInfo();
    
    // Playback control
    void stopPlayback();
    void playTestTone(uint16_t frequency = 440);
    
    // MP3 playback
    void sendMP3Data(uint8_t* data, size_t len);
    bool isReadyForData();
    void setSampleRate(uint16_t rate);
    
    // Volume control (0-100, where 100 is loudest)
    void setVolume(uint8_t volume);

private:
    uint8_t _cs, _dcs, _dreq, _rst;
    
    void writeRegister(uint8_t reg, uint16_t value);
    uint16_t readRegister(uint8_t reg);
    void writeData(uint8_t data);
    void softReset();
};

#endif // VS1053_MODULE_H