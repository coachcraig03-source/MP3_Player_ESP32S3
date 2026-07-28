// =====================================================================
//  VS1053_Module.cpp - VS1053 Implementation
// =====================================================================

#include "VS1053_Module.h"
#include <SPI.h>

// VS1053 Register definitions
#define SCI_MODE        0x00
#define SCI_STATUS      0x01
#define SCI_BASS        0x02
#define SCI_CLOCKF      0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA      0x05
#define SCI_WRAM        0x06
#define SCI_WRAMADDR    0x07
#define SCI_HDAT0       0x08
#define SCI_HDAT1       0x09
#define SCI_AIADDR      0x0A
#define SCI_VOL         0x0B
#define SCI_AICTRL0     0x0C
#define SCI_AICTRL1     0x0D
#define SCI_AICTRL2     0x0E
#define SCI_AICTRL3     0x0F

// SPI pins
#define SPI1_SCK  12
#define SPI1_MISO 13
#define SPI1_MOSI 11

VS1053_Module::VS1053_Module(uint8_t cs, uint8_t dcs, uint8_t dreq, uint8_t rst)
    : _cs(cs), _dcs(dcs), _dreq(dreq), _rst(rst)
{
    _resetMutex = xSemaphoreCreateMutex();
}

void VS1053_Module::setVolume(uint8_t volume) {
    uint8_t vs1053_vol = map(volume, 0, 100, 0x50, 0x00);
    uint16_t vol_stereo = (vs1053_vol << 8) | vs1053_vol;

    writeRegister(SCI_VOL, vol_stereo);
    
    Serial.printf("VS1053: Volume set to %d%%\n", volume);
    delay(10);
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    SPI.transfer(0x03);
    SPI.transfer(SCI_VOL);
    uint16_t readBack = SPI.transfer(0x00) << 8;
    readBack |= SPI.transfer(0x00);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    
    Serial.printf("VS1053: Volume readback = 0x%04X\n", readBack);
}

void VS1053_Module::setSampleRate(uint16_t rate) {
    writeRegister(0x05, rate);
    Serial.printf("VS1053: Sample rate set to %d Hz\n", rate);

    _sendCount = 0;
    _slowWaitCount = 0;
    _worstWaitUs = 0;
    _totalWaitUs = 0;
}

void VS1053_Module::begin() {
    Serial.println("VS1053: Initializing...");
    
    pinMode(_cs, OUTPUT);
    pinMode(_dcs, OUTPUT);
    pinMode(_dreq, INPUT);
    pinMode(_rst, OUTPUT);
    
    digitalWrite(_cs, HIGH);
    digitalWrite(_dcs, HIGH);
    
    digitalWrite(_rst, LOW);
    delay(100);
    digitalWrite(_rst, HIGH);
    delay(100);
    Serial.println("VS1053: Hardware reset complete");
    
    Serial.println("VS1053: Waiting for DREQ...");
    int timeout = 0;
    while (!digitalRead(_dreq) && timeout < 1000) {
        delay(10);
        timeout++;
    }
    
    if (digitalRead(_dreq)) {
        Serial.println("VS1053: ✓ DREQ is HIGH - chip is ready");
    } else {
        Serial.println("VS1053: ✗ DREQ timeout!");
        return;
    }

    Serial.println("VS1053: Sending software reset...");
    softReset();
    delay(100);
    
    writeRegister(0x02, 0x0000);
    writeRegister(SCI_CLOCKF, 0x8800);

    uint16_t sampleRate = readRegister(0x05);
    Serial.printf("VS1053 Sample Rate BEFORE: %d Hz\n", sampleRate);

    writeRegister(0x05, 0xAC44);
    delay(10);
    
    sampleRate = readRegister(0x05);
    Serial.printf("VS1053 Sample Rate AFTER: %d Hz\n", sampleRate);
    
    Serial.println("VS1053: ✓ Initialization complete");
}

bool VS1053_Module::isAlive() {
    uint16_t status = readRegister(SCI_STATUS);
    Serial.printf("VS1053: ✓ STATUS register: 0x%04X\n", status);
    return (status != 0x0000 && status != 0xFFFF);
}

void VS1053_Module::getChipInfo() {
    Serial.println("=== VS1053 Chip Information ===");
    Serial.printf("MODE:        0x%04X\n", readRegister(SCI_MODE));
    Serial.printf("STATUS:      0x%04X\n", readRegister(SCI_STATUS));
    Serial.printf("CLOCKF:      0x%04X\n", readRegister(SCI_CLOCKF));
    Serial.printf("VOLUME:      0x%04X\n", readRegister(SCI_VOL));
    
    uint16_t status = readRegister(SCI_STATUS);
    uint8_t version = (status >> 4) & 0x0F;
    Serial.printf("Chip version: %d\n", version);
    Serial.println("================================");
}

void VS1053_Module::playTestTone(uint16_t frequency) {
    Serial.printf("VS1053: Starting %dHz tone\n", frequency);
    
    SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
    delay(10);
    
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    SPI.transfer(0x02);
    SPI.transfer(0x00);
    SPI.transfer(0x08);
    SPI.transfer(0x24);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    delay(10);
    
    while (!digitalRead(_dreq)) delay(1);
    
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_dcs, LOW);
    SPI.transfer(0x53);
    SPI.transfer(0xEF);
    SPI.transfer(0x6E);
    SPI.transfer(frequency & 0xFF);
    SPI.transfer((frequency >> 8) & 0xFF);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    digitalWrite(_dcs, HIGH);
    SPI.endTransaction();
    
    Serial.println("VS1053: Tone started (continuous)");
}

void VS1053_Module::stopPlayback() {
    Serial.println("VS1053: Stopping playback");
    
    waitDREQ(200);
    
    SPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
    digitalWrite(_dcs, LOW);
    SPI.transfer(0x45);
    SPI.transfer(0x78);
    SPI.transfer(0x69);
    SPI.transfer(0x74);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.transfer(0x00);
    digitalWrite(_dcs, HIGH);
    SPI.endTransaction();
    
    delay(10);
    
    uint16_t mode = readRegister(SCI_MODE);
    writeRegister(SCI_MODE, mode & ~0x0020);
}

bool VS1053_Module::waitDREQ(unsigned long timeoutMs) {
    unsigned long start = millis();
    while (!digitalRead(_dreq)) {
        delay(1);
        if (millis() - start > timeoutMs) {
            Serial.println("VS1053: DREQ wait timeout (writeRegister/readRegister)");
            return false;
        }
    }
    return true;
}

void VS1053_Module::writeRegister(uint8_t reg, uint16_t value) {
    waitDREQ(200);
    
    SPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    SPI.transfer(0x02);
    SPI.transfer(reg);
    SPI.transfer(value >> 8);
    SPI.transfer(value & 0xFF);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

uint16_t VS1053_Module::readRegister(uint8_t reg) {
    waitDREQ(200);
    
    SPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    SPI.transfer(0x03);
    SPI.transfer(reg);
    uint16_t value = SPI.transfer(0x00) << 8;
    value |= SPI.transfer(0x00);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    
    return value;
}

void VS1053_Module::softReset() {
    xSemaphoreTake(_resetMutex, portMAX_DELAY);

    writeRegister(SCI_MODE, 0x0804);
    delay(100);

    unsigned long startWait = millis();
    while (!digitalRead(_dreq)) {
        delay(1);
        if (millis() - startWait > 200) {
            Serial.println("VS1053: softReset() DREQ timeout - continuing anyway");
            break;
        }
    }

    xSemaphoreGive(_resetMutex);
}

void VS1053_Module::resetForNextTrack() {
    xSemaphoreTake(_resetMutex, portMAX_DELAY);

    Serial.println("VS1053: Resetting decoder for next track");

    // Inlined reset sequence (rather than calling softReset()) so the
    // whole thing - reset, DREQ wait, AND the CLOCKF/bass restore - happens
    // under a single mutex hold. Calling softReset() here would release
    // the mutex right after its own DREQ wait, leaving a gap where the
    // other core could jump in before CLOCKF gets restored.
    writeRegister(SCI_MODE, 0x0804);
    delay(100);

    unsigned long startWait = millis();
    while (!digitalRead(_dreq)) {
        delay(1);
        if (millis() - startWait > 200) {
            Serial.println("VS1053: resetForNextTrack() DREQ timeout - continuing anyway");
            break;
        }
    }

    writeRegister(0x02, 0x0000);        // bass/treble back to neutral
    writeRegister(SCI_CLOCKF, 0x8800);  // restore 3.5x clock multiplier

    xSemaphoreGive(_resetMutex);
}

bool VS1053_Module::isReadyForData() {
    return digitalRead(_dreq) == HIGH;
}

void VS1053_Module::sendMP3Data(uint8_t* data, size_t len) {
    size_t sent = 0;
    unsigned long chunkWaitTotal = 0;
    unsigned long chunkWaitWorst = 0;

    while (sent < len) {
        unsigned long startWait = millis();
        unsigned long startWaitUs = micros();
        while (!digitalRead(_dreq)) {
            vTaskDelay(1);
            
            if (millis() - startWait > 100) {
                Serial.println("VS1053: DREQ timeout");
                return;
            }
        }
        unsigned long waitUs = micros() - startWaitUs;
        chunkWaitTotal += waitUs;
        if (waitUs > chunkWaitWorst) chunkWaitWorst = waitUs;
        
        size_t chunkSize = min((size_t)32, len - sent);
        
        SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
        digitalWrite(_dcs, LOW);
        for (size_t i = 0; i < chunkSize; i++) {
            SPI.transfer(data[sent + i]);
        }
        digitalWrite(_dcs, HIGH);
        SPI.endTransaction();
        
        sent += chunkSize;
    }

    _sendCount++;
    _totalWaitUs += chunkWaitTotal;
}