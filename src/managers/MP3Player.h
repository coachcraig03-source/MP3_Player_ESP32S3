// =====================================================================
//  MP3Player.h - Non-blocking MP3 playback state machine
// =====================================================================

#ifndef MP3_PLAYER_H
#define MP3_PLAYER_H

#include <Arduino.h>

class SD_Module;
class VS1053_Module;

enum PlaybackState {
    IDLE,
    PLAYING,
    PAUSED,
    STOPPED
};

class MP3Player {
public:
    MP3Player(SD_Module& sd, VS1053_Module& audio);
    
    // Start playing a file
    bool play(const char* path);
    
    // Control
    void pause();
    void resume();
    void stop();
    
    // Update (call from main loop)
    void update();

    
    // Status
    bool isPlaying() const { return state == PLAYING; }
    bool hasEnded() const { return state == IDLE && !needsOpen; } 
    bool isPaused() const { return state == PAUSED; }
    PlaybackState getState() const { return state; }

private:
    char pendingPath[128];
    bool needsOpen;
    SD_Module& sdModule;
    VS1053_Module& audioModule;
    
    PlaybackState state;
    uint8_t buffer[2048];  // Chunk buffer
    
    void streamChunk();
};

#endif // MP3_PLAYER_H
