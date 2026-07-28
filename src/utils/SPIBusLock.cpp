// =====================================================================
//  SPIBusLock.cpp
// =====================================================================

#include "SPIBusLock.h"

SemaphoreHandle_t spi1BusMutex = nullptr;

void initSPIBusLock() {
    spi1BusMutex = xSemaphoreCreateRecursiveMutex();
}
