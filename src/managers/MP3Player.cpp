// =====================================================================
//  MP3Player.cpp - Non-blocking MP3 playback implementation
// =====================================================================

#include "MP3Player.h"
#include "../utils/SD_Module.h"
#include "../utils/VS1053_Module.h"

MP3Player::MP3Player(SD_Module& sd, VS1053_Module& audio)
    : sdModule(sd), audioModule(audio), state(IDLE), needsOpen(false)
{
}

bool MP3Player::play(const char* path) {
    strncpy(pendingPath, path, sizeof(pendingPath));
    needsOpen = true;
    return true;
}

void MP3Player::pause() {
    if (state == PLAYING) {
        state = PAUSED;
        Serial.println("MP3Player: Paused");
    }
}

void MP3Player::resume() {
    if (state == PAUSED) {
        state = PLAYING;
        Serial.println("MP3Player: Resumed");
    }
}

void MP3Player::stop() {
    if (state != IDLE) {
        sdModule.closeFile();
        audioModule.stopPlayback();
        state = IDLE;
        Serial.println("MP3Player: Stopped");
    }
}




void MP3Player::update() {
    // Handle file opening on this core
    if (needsOpen) {
        needsOpen = false;
        if (sdModule.openFile(pendingPath)) {
            Serial.printf("MP3Player: Starting playback\n");
            state = PLAYING;
        }
        return;
    }
    
    if (state != PLAYING) return;
    
    for (int i = 0; i < 32; i++) {
        streamChunk();
        if (state != PLAYING) break;
    }
}

void MP3Player::streamChunk() {
    size_t bytesRead = sdModule.readChunk(buffer, sizeof(buffer));
    
    if (bytesRead == 0) {
        stop();
        return;
    }
    
    audioModule.sendMP3Data(buffer, bytesRead);
}