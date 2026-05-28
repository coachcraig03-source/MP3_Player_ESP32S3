// =====================================================================
//  PN532_Module.cpp - PN532 NFC Reader Implementation (I2C)
// =====================================================================

#include "PN532_Module.h"
#include "../managers/ScreenManager.h"
#include "../screens/KidScreen.h"

extern void touchISR();
// Constructor - I2C mode
PN532_Module::PN532_Module() : nfc(-1, -1) {
}

void PN532_Module::begin() {
    Serial.println("PN532: Initializing...");
    
    nfc.begin();
    
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("PN532: ✗ Not found - check wiring");
        return;
    }
    
    Serial.printf("PN532: Firmware version %d.%d ✓\n",
        (versiondata>>16) & 0xFF,
        (versiondata>>8) & 0xFF);
    
    nfc.SAMConfig();
    Serial.println("PN532: ✓ Ready!");
}

void PN532_Module::monitorForTags(ScreenManager& screenManager) {
    static bool inCardSession = false;
    static bool albumNotFound = false;
    static unsigned long noReadCount = 0;
    static unsigned long lastCheck = 0;
    
    const unsigned long CHECK_INTERVAL = inCardSession ? 1000 : 500;
    const unsigned long NO_READ_THRESHOLD = 4;

    // Don't monitor when in settings or write tag screens
    if (screenManager.isOnSettingsScreen() || 
        screenManager.isOnWriteTagScreen()) return;
    
    if (millis() - lastCheck < CHECK_INTERVAL) return;
    lastCheck = millis();

    // Detach touch interrupt during NFC read
    detachInterrupt(digitalPinToInterrupt(TOUCH_INT));
    
    bool cardFound = nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A, _uid, &_uidLength, 100);
    
    // Reattach touch interrupt
    pinMode(TOUCH_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);

    if (cardFound) {
        noReadCount = 0;
        
        if (!inCardSession) {
            Serial.println("NFC: Card detected");
            inCardSession = true;
            albumNotFound = false;
            digitalWrite(BT_ENABLE_PIN, LOW);
            
            char albumText[40];
            if (readAlbumText(albumText, sizeof(albumText))) {
                Serial.printf("NFC: Album = '%s'\n", albumText);
                screenManager.showKids();
                screenManager.getKidScreen()->showAlbum(albumText);
                
                if (!screenManager.getKidScreen()->isAlbumLoaded()) {
                    Serial.println("NFC: Album not found on SD card");
                    albumNotFound = true;
                    // Stay in session - wait for card removal to clear screen
                }
            } else {
                Serial.println("NFC: Could not read album text");
                albumNotFound = true;
                // Stay in session - wait for card removal
            }
        }
        // If already in session (found or not found) - do nothing
        
    } else if (inCardSession) {
        noReadCount++;
        if (noReadCount >= NO_READ_THRESHOLD) {
            Serial.println("NFC: Card removed");
            inCardSession = false;
            albumNotFound = false;
            noReadCount = 0;
            // Always clear and return to splash on removal
            screenManager.getKidScreen()->clearAlbum();
            screenManager.showSplash();
        }
    }
}

bool PN532_Module::isCardPresent() {
    detachInterrupt(TOUCH_INT);
    bool result = nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A, _uid, &_uidLength, 100);
    pinMode(TOUCH_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);
    return result;
}

void PN532_Module::haltCard() {
    // PN532 handles this automatically
}

bool PN532_Module::waitForCard(uint32_t timeout_ms) {
    uint32_t start = millis();
    while (millis() - start < timeout_ms) {
        detachInterrupt(digitalPinToInterrupt(TOUCH_INT));
        bool found = nfc.readPassiveTargetID(
            PN532_MIFARE_ISO14443A, _uid, &_uidLength, 100);
        pinMode(TOUCH_INT, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(TOUCH_INT), touchISR, FALLING);
        if (found) return true;
        delay(10);
    }
    return false;
}
bool PN532_Module::readUserData(uint8_t *buffer, uint8_t numBytes) {
    uint8_t startPage = 4;
    uint8_t numPages = (numBytes + 3) / 4;
    
    for (uint8_t i = 0; i < numPages; i++) {
        uint8_t readBuffer[4];
        
        if (!nfc.ntag2xx_ReadPage(startPage + i, readBuffer)) {
            Serial.printf("PN532: Read failed at page %d\n", startPage + i);
            return false;
        }
        
        for (uint8_t j = 0; j < 4 && (i*4 + j) < numBytes; j++) {
            buffer[i*4 + j] = readBuffer[j];
        }
    }
    return true;
}

int PN532_Module::extractText(const uint8_t *ndefData, char *textOut, uint8_t maxLen) {
    // Identical to RC522 version - NDEF format unchanged
    if (ndefData[0] != 0x03) return 0;
    if (ndefData[5] != 0x54) return 0;
    
    int textStart = 9;
    int textLen = 0;
    
    for (int i = textStart; i < maxLen + textStart && 
         ndefData[i] != 0x00 && ndefData[i] != 0xFE; i++) {
        if (ndefData[i] >= 32 && ndefData[i] <= 126) {
            textOut[textLen++] = ndefData[i];
        }
    }
    
    textOut[textLen] = '\0';
    return textLen;
}

bool PN532_Module::readAlbumText(char *textOut, uint8_t maxLen) {
    uint8_t userData[48];
    
    if (!readUserData(userData, 48)) {
        return false;
    }
    
    int textLen = extractText(userData, textOut, maxLen);
    return textLen > 0;
}

bool PN532_Module::writeAlbumTag(const char* albumName) {
    uint8_t message[48] = {0};
    
    message[0] = 0x03;
    message[1] = strlen(albumName) + 7;
    message[2] = 0xD1;
    message[3] = 0x01;
    message[4] = strlen(albumName) + 3;
    message[5] = 0x54;
    message[7] = 0x02;
    message[8] = 'e';
    message[9] = 'n';
    strcpy((char*)&message[9], albumName);
    message[10 + strlen(albumName)] = 0xFE;
    
    for (int page = 4; page < 16; page++) {
        uint8_t block[4];
        memcpy(block, &message[(page-4)*4], 4);
        
        if (!nfc.ntag2xx_WritePage(page, block)) {
            Serial.printf("PN532: Write failed at page %d\n", page);
            return false;
        }
    }
    return true;
}

void PN532_Module::runTest(int count) {
    for (int i = 0; i < count; i++) {
        Serial.printf("\n--- PN532 Read %d/%d ---\n", i + 1, count);
        Serial.println("Waiting for card...");
        
        if (!waitForCard(5000)) {
            Serial.println("No card found (timeout)");
            continue;
        }
        
        Serial.print("UID: ");
        for (byte j = 0; j < _uidLength; j++) {
            Serial.printf("%02X ", _uid[j]);
        }
        Serial.println();
        
        char cleanText[40];
        if (readAlbumText(cleanText, sizeof(cleanText) - 1)) {
            Serial.printf("Album: \"%s\"\n", cleanText);
        } else {
            Serial.println("Failed to read album text");
        }
        
        delay(500);
    }
}