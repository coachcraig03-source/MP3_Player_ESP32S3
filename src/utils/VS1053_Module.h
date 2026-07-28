// =====================================================================
//  VS1053_Module.h - VS1053 Audio Module
// =====================================================================

#ifndef VS1053_MODULE_H
#define VS1053_MODULE_H

#include <Arduino.h>
#include <freertos/semphr.h>

class VS1053_Module {
public:
    VS1053_Module(uint8_t cs, uint8_t dcs, uint8_t dreq, uint8_t rst);
    
    void begin();
    bool isAlive();
    void getChipInfo();
    void softReset();
    void resetForNextTrack();

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
    bool waitDREQ(unsigned long timeoutMs);  // shared, timeout-protected DREQ wait

    // Serializes reset sequences between cores. MP3Player::stop() runs on
    // Core 0 (natural end-of-track) and calls resetForNextTrack(); user
    // album selection runs on Core 1 and calls softReset() directly. Both
    // are multi-step sequences (register write, delay, DREQ wait, more
    // register writes) that are NOT atomic as a whole even though each
    // individual SPI transfer is protected - if both cores start a reset
    // sequence close together, they can interleave and leave the chip in
    // a state where DREQ stops behaving predictably. This mutex makes one
    // core wait for the other to finish its full reset before starting
    // its own.
    SemaphoreHandle_t _resetMutex;

    // --- instrumentation only, no behavioral effect ---
    unsigned long _sendCount = 0;
    unsigned long _slowWaitCount = 0;
    unsigned long _worstWaitUs = 0;
    unsigned long _totalWaitUs = 0;
};

#endif // VS1053_MODULE_H