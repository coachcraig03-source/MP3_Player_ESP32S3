// =====================================================================
//  SPIBusLock.h - Shared lock for the physical SPI1 bus
//
//  SD_Module and VS1053_Module both live on SPI1 (same SCK/MOSI/MISO,
//  different CS pins). mp3StreamTask runs on Core 0 and hits VS1053
//  constantly during playback; touch/NFC handling runs on Core 1 and can
//  trigger SD_Module calls (album browsing, NFC-triggered showAlbum(),
//  which itself calls SD file open/close) at any time. The Arduino SPI
//  driver's own internal locking is not safe against this - it produced
//  a hard FreeRTOS assert crash (xQueueGenericSend) when SD_Module's
//  FsBaseFile::close() on Core 1 landed at the same moment VS1053's
//  sendMP3Data() was mid-transaction on Core 0.
//
//  This mutex is RECURSIVE because softReset()/resetForNextTrack() need
//  to hold the lock across their whole multi-step sequence (several
//  register writes + DREQ waits) while internally calling writeRegister()
//  /readRegister(), which also take the lock on their own when called
//  directly. A plain (non-recursive) mutex would deadlock on that nesting.
// =====================================================================

#ifndef SPI_BUS_LOCK_H
#define SPI_BUS_LOCK_H

#include <Arduino.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t spi1BusMutex;

// Call once, early in setup(), before SD_Module::begin() or
// VS1053_Module::begin() are called.
void initSPIBusLock();

// RAII guard - takes the lock on construction, releases on destruction
// (including on early return from whatever function declares one). This
// is deliberately simple so every SPI1-touching function can protect
// itself with a single line at the top: `SPIBusGuard guard;`
struct SPIBusGuard {
    SPIBusGuard() {
        xSemaphoreTakeRecursive(spi1BusMutex, portMAX_DELAY);
    }
    ~SPIBusGuard() {
        xSemaphoreGiveRecursive(spi1BusMutex);
    }
};

#endif // SPI_BUS_LOCK_H
