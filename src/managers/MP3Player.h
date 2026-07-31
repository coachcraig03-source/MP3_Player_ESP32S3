// =====================================================================
//  MP3Player.h - Non-blocking MP3 playback state machine
//  Now with a small read-ahead ring buffer instead of a single chunk
//  buffer, to absorb occasional slow SD reads without an audible gap.
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
    volatile bool stopRequested = false;
    
    // Control
    void pause();
    void resume();
    void stop();
    
    // Update (call from main loop)
    void update();
    void requestStop();

    
    // Status
    bool isPlaying() const { return state == PLAYING; }
    bool hasEnded() const { return state == IDLE && !needsOpen; } 
    bool isPaused() const { return state == PAUSED; }
    PlaybackState getState() const { return state; }

    // True exactly once, only when playback reached the real end of
    // the file (readChunk() returned 0) - as opposed to state merely
    // being IDLE, which is also briefly true during ANY deliberate
    // stop-then-restart (Back button, screen switch, NFC tag, manual
    // track change). Auto-advance logic should gate on this, not on
    // isPlaying()/hasEnded() - both of those go through the same
    // transient IDLE window a deliberate interruption produces, and
    // anything polling continuously (main loop(), a screen's update())
    // can catch that window and misfire mid-transition. Clears itself
    // on read, and is also cleared by play()/requestStop(), so a
    // deliberate interruption can never be mistaken for a real end.
    bool consumeNaturalEnd();

private:
    static const size_t CHUNK_SIZE = 2048;
    static const int QUEUE_DEPTH = 4;   // ~4 x 2048 bytes = ~46ms of audio banked ahead

    char pendingPath[128];
    volatile bool needsOpen;
    SD_Module& sdModule;
    VS1053_Module& audioModule;
    
    // Both read on Core 1 (main loop(), MP3SongList::update() polling
    // hasEnded()) and written on Core 0 (mp3StreamTask calling
    // update()) - same cross-core relationship stopRequested already
    // needed volatile for.
    volatile PlaybackState state;
    volatile bool naturalEnd;   // see consumeNaturalEnd() above

    // Ring buffer of pre-read chunks
    uint8_t chunkBuf[QUEUE_DEPTH][CHUNK_SIZE];
    size_t  chunkLen[QUEUE_DEPTH];
    int     queueHead;    // index of the oldest filled chunk (next to send)
    int     queueTail;    // index of the next empty slot (next to fill)
    int     queueCount;   // how many filled, unsent chunks are queued
    bool    eofReached;   // true once readChunk() has returned 0 for this file

    void fillQueue();        // top up the ring buffer from SD, up to QUEUE_DEPTH
    bool sendNextQueued();   // send the oldest queued chunk, if any; false if queue was empty
    void resetQueue();       // called on play() / stop()
};

#endif // MP3_PLAYER_H
