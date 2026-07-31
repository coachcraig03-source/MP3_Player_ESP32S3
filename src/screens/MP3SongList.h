// =====================================================================
//  MP3SongList.h - Now Playing / song list screen
// =====================================================================

#ifndef MP3_SONG_LIST_H
#define MP3_SONG_LIST_H

#include "../managers/BaseScreen.h"
#include "../ui/UIButton.h"
#include "../ui/UISlider.h"

class ScreenManager;
class TFT_Module;
class SD_Module;
class VS1053_Module;

class MP3SongList : public BaseScreen {
public:
    MP3SongList(ScreenManager& manager, TFT_Module& tft, SD_Module& sd, VS1053_Module& audio);

    void begin() override;
    void update() override;
    void handleTouch(int x, int y) override;

    // Called by MP3AlbumList right before navigating here, so this
    // screen knows which album's tracks to load.
    void loadAlbum(const char* albumName);

    // Called by ScreenManager::handleSongEnd() when the current track
    // finishes naturally (main.cpp's loop() owns detecting that via
    // MP3Player::consumeNaturalEnd() - it's a one-shot signal, so
    // exactly one consumer gets it, and ScreenManager::handleSongEnd()
    // is that single dispatch point for every screen, not just this one).
    void advanceToNextTrack();

private:
    static const int MAX_TRACKS = 100;       // matches the old MP3Screen's cap
    static const int VISIBLE_TRACK_ROWS = 12;

    void drawScreen();             // full redraw - only called from begin()
    void updateNowPlaying();       // partial redraw for track changes - title + track list only, does NOT touch album art
    void drawTrackListArea();      // partial redraw - just the track list + its scroll slider
    void drawTitle();              // partial redraw - just the now-playing title
    void updateScrollOffsetFromSlider();
    void playTrack(int index);

    // Art loading is split in two on purpose. loadAlbumArt() reads +
    // decodes from SD into artBuffer (RAM) and MUST be called before
    // playback starts - it holds the SPI1 bus guard for the whole
    // decode, which is long enough to starve Core 0's audio streaming
    // if a track is already playing. blitAlbumArt() just pushes the
    // already-decoded buffer to the TFT (a different SPI bus, no SPI1
    // contention at all) and is safe to call any time, including from
    // drawScreen() after playback has started.
    void loadAlbumArt();           // SD read + JPEG decode -> artBuffer. Call BEFORE playTrack().
    void blitAlbumArt();           // artBuffer -> TFT. Cheap, no SD/SPI1 involvement.
    void drawAlbumArtPlaceholder();

    char currentAlbumName[64];
    char trackNames[MAX_TRACKS][64];
    int trackCount;

    int scrollOffset;
    int maxScrollOffset;    // real, per-album usable scroll range (trackCount - VISIBLE_TRACK_ROWS)
    int sliderMaxValue;     // fixed at construction, the slider's own worst-case range
    int currentTrackIndex;  // index of the track actually playing (or about to play)
    bool isPlaying;
    bool albumArtLoaded;    // true if artBuffer currently holds a valid decoded image for currentAlbumName

    uint16_t* artBuffer;    // ART_SIZE x ART_SIZE RGB565, allocated once (prefers PSRAM), reused/overwritten per album

    SD_Module& sdModule;
    VS1053_Module& audioModule;

    UIButton backButton;
    UIButton prevButton;
    UIButton playPauseButton;
    UIButton nextButton;
    UISlider volumeSlider;
    UISlider trackScrollSlider;
};

#endif // MP3_SONG_LIST_H
