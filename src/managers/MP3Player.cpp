// =====================================================================
//  MP3Player.cpp - Non-blocking MP3 playback implementation
// =====================================================================

#include "MP3Player.h"
#include "../utils/SD_Module.h"
#include "../utils/VS1053_Module.h"

MP3Player::MP3Player(SD_Module& sd, VS1053_Module& audio)
    : sdModule(sd), audioModule(audio), state(IDLE)
{
}

bool MP3Player::play(const char* path) {
    stop();
    
    if (!sdModule.openFile(path)) {
        return false;
    }
    
    // Reset VS1053 to ensure clean state
    //audioModule.softReset();  // Make this public in VS1053_Module.h
    delay(100);
    
    Serial.printf("MP3Player: Starting playback of %s\n", path);
    state = PLAYING;
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
    if (state != PLAYING) return;
    
    //Serial.println("MP3Player::update() running"); // Add this
    
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