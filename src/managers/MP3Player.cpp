// =====================================================================
//  MP3Player.cpp - Non-blocking MP3 playback implementation
//  Read-ahead ring buffer version.
// =====================================================================

#include "MP3Player.h"
#include "../utils/SD_Module.h"
#include "../utils/VS1053_Module.h"
#include "pins.h"

MP3Player::MP3Player(SD_Module& sd, VS1053_Module& audio)
    : sdModule(sd), audioModule(audio), state(IDLE), needsOpen(false),
      queueHead(0), queueTail(0), queueCount(0), eofReached(false),
      naturalEnd(false)
{
}

void MP3Player::resetQueue() {
    queueHead = 0;
    queueTail = 0;
    queueCount = 0;
    eofReached = false;
}

bool MP3Player::play(const char* path) {
    // A fresh play() call always supersedes any stop that hasn't been
    // serviced yet. Without this, a stopRequested left over from
    // whatever was happening a moment ago (e.g. leaving a different
    // screen) can land in the same update() tick as this new track's
    // needsOpen, producing a transient stop+reopen right as playback
    // starts - long enough for anything polling end-of-track state to
    // misread it as "already finished" and skip straight to the next
    // track before this one ever really began.
    stopRequested = false;
    naturalEnd = false;

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
        audioModule.resetForNextTrack();
        state = IDLE;
        resetQueue();
        Serial.println("MP3Player: Stopped");
    }
}

void MP3Player::requestStop() {
    naturalEnd = false;
    stopRequested = true;
}

void MP3Player::fillQueue() {
    // Top up the ring buffer with freshly-read chunks, up to QUEUE_DEPTH.
    // Each readChunk() call is already protected by SD_Module's own bus
    // lock, so this is safe to call every update() cycle.
    while (queueCount < QUEUE_DEPTH && !eofReached) {
        size_t bytesRead = sdModule.readChunk(chunkBuf[queueTail], CHUNK_SIZE);

        if (bytesRead == 0) {
            eofReached = true;
            break;
        }

        chunkLen[queueTail] = bytesRead;
        queueTail = (queueTail + 1) % QUEUE_DEPTH;
        queueCount++;
    }
}

bool MP3Player::sendNextQueued() {
    if (queueCount == 0) {
        return false;
    }

    audioModule.sendMP3Data(chunkBuf[queueHead], chunkLen[queueHead]);

    queueHead = (queueHead + 1) % QUEUE_DEPTH;
    queueCount--;
    return true;
}

void MP3Player::update() {
    if (stopRequested) {
        stopRequested = false;
        stop();
        // Don't return - fall through to check needsOpen
    }
    
    if (needsOpen) {
        // needsOpen is cleared at the END of this block now, not the
        // start. Clearing it first left a real window - however brief -
        // where needsOpen was already false but state hadn't been set
        // to PLAYING yet (openFile()/setSampleRate() take actual time:
        // an SD open plus a VS1053 register write with its own DREQ
        // wait). hasEnded() is state==IDLE && !needsOpen - during that
        // window both were true, with no track having actually ended.
        // Usually too narrow to matter, but MP3SongList polls
        // hasEnded() every loop tick on the other core, so occasionally
        // the timing lined up and it read "ended" moments after a track
        // had just started, skipping straight to the next one.
        resetQueue();
        if (sdModule.openFile(pendingPath)) {
            Serial.printf("MP3Player: Starting playback\n");
            audioModule.setSampleRate(44100);
            SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
            delay(5);
            state = PLAYING;

            // Prime with a couple of chunks before the first send, so we
            // start with a small cushion rather than from completely empty.
            fillQueue();
        }
        needsOpen = false;
        return;
    }
    
    if (state != PLAYING) return;

    // Interleave one fill-attempt and one send per iteration, same
    // cadence as the original read-then-send code - NOT a batch of reads
    // followed by a batch of sends. Batching reads first (the previous
    // version of this file) created a bigger single gap in feeding the
    // VS1053 than the original code had, since its internal buffer can
    // only bridge very short gaps. This version still keeps a standing
    // backlog (up to QUEUE_DEPTH) for cushioning against an occasional
    // slow individual read, without ever pausing sends to do several
    // reads in a row.
    for (int i = 0; i < 32; i++) {
        // Top up by at most ONE slot per iteration, only if there's room.
        if (queueCount < QUEUE_DEPTH && !eofReached) {
            size_t bytesRead = sdModule.readChunk(chunkBuf[queueTail], CHUNK_SIZE);
            if (bytesRead == 0) {
                eofReached = true;
            } else {
                chunkLen[queueTail] = bytesRead;
                queueTail = (queueTail + 1) % QUEUE_DEPTH;
                queueCount++;
            }
        }

        // Send the oldest queued chunk, if any.
        if (!sendNextQueued()) {
            if (eofReached) {
                // This is the ONLY place a track end is real - readChunk()
                // genuinely returned 0, nothing left to send. Distinct
                // from state just being IDLE, which is also true during
                // any deliberate stop-then-restart.
                naturalEnd = true;
                stop();
            }
            break;
        }
        if (state != PLAYING) break;
    }
}

bool MP3Player::consumeNaturalEnd() {
    if (naturalEnd) {
        naturalEnd = false;
        return true;
    }
    return false;
}
