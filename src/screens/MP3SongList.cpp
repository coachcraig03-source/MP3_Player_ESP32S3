// =====================================================================
//  MP3SongList.cpp - Now Playing / song list screen
// =====================================================================

#include "MP3SongList.h"
#include "../managers/ScreenManager.h"
#include "../utils/TFT_Module.h"
#include "../utils/SD_Module.h"
#include "../utils/VS1053_Module.h"
#include "../utils/SPIBusLock.h"
#include "../managers/MP3Player.h"
#include <LovyanGFX.hpp>
#include <lgfx/v1/lgfx_fonts.hpp>
#include <SdFat.h>
#include <TJpg_Decoder.h>
#include <esp_heap_caps.h>

#define ART_X       10
#define ART_Y       55
#define ART_SIZE    160

#define TRACK_X       185
#define TRACK_Y_START 55
#define TRACK_ROW_H   16
#define TRACK_AREA_H  (VISIBLE_TRACK_ROWS * TRACK_ROW_H)

#define TITLE_Y            12
#define TITLE_MAX_WIDTH_PX 280   // safe width that won't overlap Back (ends at x=90) when centered at x=240

// Row tap area right edge - stops short of trackScrollSlider (x=405) so
// the slider's own hit test still gets first shot at that strip.
#define TRACK_TAP_RIGHT_X  400

// TJpg_Decoder's callback writes into artBuffer (a RAM canvas) instead
// of straight to the TFT. Decoding happens before playback starts (see
// loadAlbum()), so nothing here touches the display or the SPI1 bus -
// x/y arrive relative to the canvas origin we pass into drawJpg(), not
// absolute screen coordinates.
static uint16_t* artDecodeTarget = nullptr;
static int artDecodeCanvasW = 0;
static int artDecodeCanvasH = 0;

static bool tftOutput_SongList_ToBuffer(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if (!artDecodeTarget || !bitmap) return false;

    for (int row = 0; row < h; row++) {
        int destY = y + row;
        if (destY < 0 || destY >= artDecodeCanvasH) continue;

        for (int col = 0; col < w; col++) {
            int destX = x + col;
            if (destX < 0 || destX >= artDecodeCanvasW) continue;
            artDecodeTarget[destY * artDecodeCanvasW + destX] = bitmap[row * w + col];
        }
    }
    return true;
}

MP3SongList::MP3SongList(ScreenManager& manager, TFT_Module& tftModule, SD_Module& sd, VS1053_Module& audio)
    : BaseScreen(manager, tftModule),
      sdModule(sd),
      audioModule(audio),
      trackCount(0),
      scrollOffset(0),
      currentTrackIndex(0),
      isPlaying(false),
      albumArtLoaded(false),
      artBuffer(nullptr),
      backButton(10, 10, 80, 40, "Back"),
      prevButton(40, 265, 60, 45, "<<"),
      playPauseButton(120, 265, 100, 45, "Play"),
      nextButton(240, 265, 60, 45, ">>"),
      volumeSlider(440, 55, 30, 200, 0, 100),
      trackScrollSlider(405, TRACK_Y_START, 15, TRACK_AREA_H, 0,
                         (MAX_TRACKS > VISIBLE_TRACK_ROWS) ? (MAX_TRACKS - VISIBLE_TRACK_ROWS) : 0)
{
    backButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    prevButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    playPauseButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    nextButton.setColors(TFT_DARKGREY, TFT_WHITE, TFT_WHITE);
    volumeSlider.setValue(75);
    trackScrollSlider.setColors(0x4208, TFT_WHITE, TFT_CYAN);

    sliderMaxValue = (MAX_TRACKS > VISIBLE_TRACK_ROWS) ? (MAX_TRACKS - VISIBLE_TRACK_ROWS) : 0;
    maxScrollOffset = 0;

    currentAlbumName[0] = '\0';

    // Allocate the art canvas once and reuse it for every album -
    // prefer PSRAM (plenty of headroom on the N8R8) and fall back to
    // internal SRAM if PSRAM isn't available for some reason.
    size_t artBytes = ART_SIZE * ART_SIZE * sizeof(uint16_t);
    artBuffer = (uint16_t*)heap_caps_malloc(artBytes, MALLOC_CAP_SPIRAM);
    if (!artBuffer) {
        Serial.println("MP3SongList: PSRAM art buffer alloc failed, trying internal RAM");
        artBuffer = (uint16_t*)malloc(artBytes);
    }
    if (!artBuffer) {
        Serial.println("MP3SongList: Art buffer alloc failed entirely - art will be disabled");
    } else {
        memset(artBuffer, 0, artBytes);
    }
}

void MP3SongList::loadAlbum(const char* albumName) {
    // Direct access to the global `sd` object, not through SD_Module's
    // own locked methods - MUST be guarded (root cause of the
    // reproducible SPI bus crash found and fixed earlier this session).
    SPIBusGuard guard;

    // Stop whatever might be playing and reset the VS1053, same as
    // KidScreen::showAlbum() has always done. This screen was missing
    // that step entirely - on a cold boot straight into SongList,
    // nothing had ever called softReset() or resetForNextTrack()
    // (state starts IDLE and stays that way until the first play()
    // succeeds, so MP3Player::stop()'s own reset path never runs
    // either), leaving the very first playback attempt relying solely
    // on setup()'s one-time boot self-test - which isn't reliable
    // enough on its own. This is what made SongList's cold-start
    // failure look "intermittent": it was actually deterministic,
    // just masked whenever NFC (which does reset unconditionally) or
    // any other prior playback attempt happened first.
    extern MP3Player mp3Player;
    mp3Player.requestStop();
    delay(200);
    audioModule.softReset();
    delay(100);

    strncpy(currentAlbumName, albumName, sizeof(currentAlbumName) - 1);
    currentAlbumName[sizeof(currentAlbumName) - 1] = '\0';

    Serial.printf("MP3SongList: Loading tracks for '%s'\n", currentAlbumName);

    trackCount = 0;
    scrollOffset = 0;

    char folderPath[128];
    snprintf(folderPath, sizeof(folderPath), "/Music/%s", currentAlbumName);

    extern SdFs sd;
    FsFile albumDir;
    if (!albumDir.open(folderPath)) {
        Serial.println("MP3SongList: Failed to open album folder");
        maxScrollOffset = 0;
        return;
    }

    FsFile file;
    while (file.openNext(&albumDir, O_RDONLY) && trackCount < MAX_TRACKS) {
        char name[64];
        file.getName(name, sizeof(name));

        if (strstr(name, ".mp3") || strstr(name, ".MP3") ||
            strstr(name, ".wma") || strstr(name, ".WMA")) {
            strncpy(trackNames[trackCount], name, sizeof(trackNames[0]) - 1);
            trackNames[trackCount][sizeof(trackNames[0]) - 1] = '\0';
            trackCount++;
        }

        file.close();
    }
    albumDir.close();

    Serial.printf("MP3SongList: Loaded %d tracks\n", trackCount);

    maxScrollOffset = trackCount - VISIBLE_TRACK_ROWS;
    if (maxScrollOffset < 0) maxScrollOffset = 0;

    // Decode art BEFORE starting playback - same reasoning as the old
    // MP3Screen's "load art first" comment. loadAlbumArt() holds the
    // SPI1 bus guard for the whole SD read + decode; doing that while
    // a track is already streaming starves Core 0 long enough to kill
    // playback entirely (confirmed by testing). Once this returns, the
    // decoded image just sits in artBuffer until drawScreen() blits it.
    loadAlbumArt();

    // Auto-play the first track, matching the old MP3Screen's behavior
    // when an album was selected.
    if (trackCount > 0) {
        playTrack(0);
    }
}

void MP3SongList::playTrack(int index) {
    if (index < 0 || index >= trackCount) return;

    currentTrackIndex = index;

    char trackPath[256];
    snprintf(trackPath, sizeof(trackPath), "/Music/%s/%s", currentAlbumName, trackNames[index]);

    Serial.printf("MP3SongList: Playing %s\n", trackPath);

    extern MP3Player mp3Player;
    mp3Player.play(trackPath);

    isPlaying = true;
    playPauseButton.setLabel("Pause");
}

void MP3SongList::updateScrollOffsetFromSlider() {
    // trackScrollSlider's raw value range (0..sliderMaxValue) is fixed
    // at construction time, sized for the worst case of MAX_TRACKS (100)
    // tracks - sliderMaxValue is 88. Most albums have far fewer tracks
    // than that, so maxScrollOffset (this album's real scrollable range)
    // is usually much smaller - 7, for a 19-track album. Directly
    // clamping rawOffset into [0, maxScrollOffset] collapsed almost the
    // entire physical drag range onto a single clamped value, leaving
    // only a sliver of travel with any real effect - that's the "jumps
    // between two states" behavior. Scale proportionally instead, so
    // the full physical slider travel always maps across the whole
    // actual scroll range for whatever album is loaded.
    int rawValue = trackScrollSlider.getValue();
    int rawOffset = sliderMaxValue - rawValue;

    if (sliderMaxValue <= 0) {
        scrollOffset = 0;
        return;
    }

    scrollOffset = (int)((long)rawOffset * maxScrollOffset / sliderMaxValue);
    scrollOffset = constrain(scrollOffset, 0, maxScrollOffset);
}

void MP3SongList::begin() {
    scrollOffset = 0;
    trackScrollSlider.setValue(sliderMaxValue);
    drawScreen();
}

void MP3SongList::drawTrackListArea() {
    auto display = tft.getTFT();

    display->fillRect(TRACK_X - 5, TRACK_Y_START, 425 - (TRACK_X - 5), TRACK_AREA_H, TFT_BLACK);

    display->setFont(&fonts::Font0);
    display->setTextSize(1);

    if (trackCount == 0) {
        display->setTextColor(TFT_DARKGREY);
        display->setTextDatum(top_left);
        display->drawString("No tracks found", TRACK_X, TRACK_Y_START);
        return;
    }

    for (int i = 0; i < VISIBLE_TRACK_ROWS; i++) {
        int trackIndex = scrollOffset + i;
        if (trackIndex >= trackCount) break;

        int rowY = TRACK_Y_START + (i * TRACK_ROW_H);
        display->setTextColor(trackIndex == currentTrackIndex ? TFT_YELLOW : TFT_WHITE);
        display->setTextDatum(middle_left);

        String trackName = String(trackNames[trackIndex]);
        if (trackName.length() > 30) {
            trackName = trackName.substring(0, 27) + "...";
        }
        display->drawString(trackName, TRACK_X, rowY + TRACK_ROW_H / 2);
    }

    if (maxScrollOffset > 0) {
        trackScrollSlider.draw(tft);
    }
}

void MP3SongList::drawTitle() {
    auto display = tft.getTFT();

    // Clear just the title strip - avoid a full screen clear for what's
    // otherwise a one-line text update.
    display->fillRect(95, 0, 465 - 95, TITLE_Y + 30, TFT_BLACK);

    display->setFont(&fonts::Font0);
    display->setTextSize(2);
    display->setTextColor(TFT_CYAN);
    display->setTextDatum(top_center);

    String title = (trackCount > 0) ? String(trackNames[currentTrackIndex]) : String(currentAlbumName);
    while (display->textWidth(title) > TITLE_MAX_WIDTH_PX && title.length() > 4) {
        title = title.substring(0, title.length() - 4) + "...";
    }
    display->drawString(title, 240, TITLE_Y);

    display->setFont(&fonts::Font0);
    display->setTextSize(1);
}

void MP3SongList::updateNowPlaying() {
    // Lightweight redraw for track changes (prev/next/tap-row). Updates
    // the title and track list only - deliberately does NOT touch the
    // album art area, since that would mean re-reading and re-decoding
    // the cover JPEG from SD on every track change.
    drawTitle();
    drawTrackListArea();
    playPauseButton.setLabel(isPlaying ? "Pause" : "Play");
    playPauseButton.draw(tft);
}

void MP3SongList::drawAlbumArtPlaceholder() {
    auto display = tft.getTFT();
    display->setFont(&fonts::Font0);
    display->setTextSize(1);
    display->setTextColor(TFT_DARKGREY);
    display->setTextDatum(middle_center);
    display->drawString("Album Art", ART_X + ART_SIZE / 2, ART_Y + ART_SIZE / 2);
}

void MP3SongList::loadAlbumArt() {
    // Called from loadAlbum(), BEFORE playTrack() starts streaming -
    // this is the important part. It holds the SPI1 bus guard for the
    // whole SD read + JPEG decode; doing that once a track is already
    // playing starves Core 0 long enough to kill playback outright
    // (confirmed by testing - the previous "decode straight to screen
    // from drawScreen()" ordering caused a total streaming failure).
    // Decoding into artBuffer here means the later on-screen blit
    // (blitAlbumArt(), called from drawScreen()) is pure RAM-to-TFT
    // with zero SD/SPI1 involvement, safe regardless of what else is
    // happening.
    SPIBusGuard guard;

    albumArtLoaded = false;

    if (!artBuffer) {
        Serial.println("MP3SongList: No art buffer available, skipping art");
        return;
    }

    size_t artBytes = ART_SIZE * ART_SIZE * sizeof(uint16_t);
    memset(artBuffer, 0, artBytes);

    // Independent FsFile, not routed through SD_Module - same reasoning
    // as before: SD_Module appears to use a single shared file handle
    // for its own streaming reads, and opening art through that path
    // stomped whatever track was playing. This runs before playback
    // starts now, but keeping the independent handle is still correct
    // and matches the track-scan code's own pattern.
    extern SdFs sd;
    char artPath[128];
    snprintf(artPath, sizeof(artPath), "/Music/%s/folder.jpg", currentAlbumName);

    FsFile artFile;
    bool opened = artFile.open(artPath, O_RDONLY);
    if (!opened) {
        Serial.println("MP3SongList: No album-specific art, trying default");
        opened = artFile.open("/Music/FolderDefault.jpg", O_RDONLY);
    }

    if (!opened) {
        Serial.println("MP3SongList: No default art found either");
        return;
    }

    uint8_t* buffer = new uint8_t[50000];
    size_t totalRead = 0;
    int bytesRead;
    while ((bytesRead = artFile.read(buffer + totalRead, 512)) > 0) {
        totalRead += bytesRead;
        if (totalRead >= 50000) break;
    }
    artFile.close();

    if (totalRead == 0) {
        Serial.println("MP3SongList: Album art file read as empty");
        delete[] buffer;
        return;
    }

    uint16_t jpgW = 0, jpgH = 0;
    if (TJpgDec.getJpgSize(&jpgW, &jpgH, buffer, totalRead) != JDR_OK || jpgW == 0 || jpgH == 0) {
        Serial.println("MP3SongList: Failed to parse JPEG header");
        delete[] buffer;
        return;
    }

    // Cover art is usually bigger than the 160x160 art box (300x300,
    // 500x500, etc. are common), and TJpg_Decoder can only downscale by
    // powers of 2 (1/2/4/8) - so the decoded output rarely lands near
    // 160x160, often leaving a lot of the box empty. Decode at the
    // decoder's native output size into a small temp buffer, then do a
    // nearest-neighbor upscale into the full art box below, preserving
    // aspect ratio, so the box actually gets filled regardless of the
    // source resolution.
    uint8_t scale = 1;
    while (scale < 8 && (jpgW / scale > ART_SIZE || jpgH / scale > ART_SIZE)) {
        scale *= 2;
    }

    int decodedW = jpgW / scale;
    int decodedH = jpgH / scale;
    if (decodedW < 1) decodedW = 1;
    if (decodedH < 1) decodedH = 1;

    size_t tempBytes = (size_t)decodedW * decodedH * sizeof(uint16_t);
    uint16_t* tempBuffer = (uint16_t*)heap_caps_malloc(tempBytes, MALLOC_CAP_SPIRAM);
    if (!tempBuffer) tempBuffer = (uint16_t*)malloc(tempBytes);
    if (!tempBuffer) {
        Serial.println("MP3SongList: Temp art buffer alloc failed, skipping art");
        delete[] buffer;
        return;
    }
    memset(tempBuffer, 0, tempBytes);

    TJpgDec.setJpgScale(scale);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tftOutput_SongList_ToBuffer);

    artDecodeTarget = tempBuffer;
    artDecodeCanvasW = decodedW;
    artDecodeCanvasH = decodedH;

    TJpgDec.drawJpg(0, 0, buffer, totalRead);

    artDecodeTarget = nullptr;
    delete[] buffer;

    // Nearest-neighbor upscale from tempBuffer (decodedW x decodedH)
    // into artBuffer (ART_SIZE x ART_SIZE) - scale to fill as much of
    // the box as possible while preserving aspect ratio, then center.
    float scaleToFitW = ART_SIZE / (float)decodedW;
    float scaleToFitH = ART_SIZE / (float)decodedH;
    float scaleToFit = (scaleToFitW < scaleToFitH) ? scaleToFitW : scaleToFitH;

    int targetW = (int)(decodedW * scaleToFit);
    int targetH = (int)(decodedH * scaleToFit);
    if (targetW > ART_SIZE) targetW = ART_SIZE;
    if (targetH > ART_SIZE) targetH = ART_SIZE;
    if (targetW < 1) targetW = 1;
    if (targetH < 1) targetH = 1;

    int offsetX = (ART_SIZE - targetW) / 2;
    int offsetY = (ART_SIZE - targetH) / 2;

    for (int y = 0; y < targetH; y++) {
        int srcY = (int)(y / scaleToFit);
        if (srcY >= decodedH) srcY = decodedH - 1;

        for (int x = 0; x < targetW; x++) {
            int srcX = (int)(x / scaleToFit);
            if (srcX >= decodedW) srcX = decodedW - 1;
            artBuffer[(offsetY + y) * ART_SIZE + (offsetX + x)] = tempBuffer[srcY * decodedW + srcX];
        }
    }

    heap_caps_free(tempBuffer);
    albumArtLoaded = true;
}

void MP3SongList::blitAlbumArt() {
    // Pure RAM-to-TFT push over HSPI/SPI2 (the TFT's own bus) - no SD
    // or SPI1 involvement, safe to call any time regardless of what
    // Core 0 is doing.
    auto display = tft.getTFT();

    if (albumArtLoaded && artBuffer) {
        display->pushImage(ART_X, ART_Y, ART_SIZE, ART_SIZE, artBuffer);
    } else {
        display->fillRect(ART_X, ART_Y, ART_SIZE, ART_SIZE, TFT_BLACK);
    }

    // Border drawn on top so it's never obscured by the image/black fill.
    display->drawRect(ART_X, ART_Y, ART_SIZE, ART_SIZE, TFT_WHITE);

    if (!albumArtLoaded) {
        drawAlbumArtPlaceholder();
    }
}

void MP3SongList::drawScreen() {
    auto display = tft.getTFT();
    display->fillScreen(TFT_BLACK);

    backButton.draw(tft);

    drawTitle();

    // Album art - artBuffer was already decoded back in loadAlbum(),
    // before playback started. This is just a fast blit.
    blitAlbumArt();

    drawTrackListArea();

    display->setFont(&fonts::Font0);
    display->setTextSize(1);
    display->setTextColor(TFT_WHITE);
    display->setTextDatum(top_center);
    display->drawString("VOL", 455, ART_Y - 15);
    volumeSlider.draw(tft);

    prevButton.draw(tft);
    playPauseButton.draw(tft);
    nextButton.draw(tft);

    display->setFont(&fonts::Font0);
}

void MP3SongList::update() {
    // Auto-advance dispatch now happens centrally through
    // ScreenManager::handleSongEnd() (called from main.cpp's loop() via
    // MP3Player::consumeNaturalEnd(), a one-shot signal) - see
    // advanceToNextTrack() below. Polling consumeNaturalEnd() here too
    // would just lose the race to loop(), which always runs first.
}

void MP3SongList::advanceToNextTrack() {
    if (!isPlaying || trackCount == 0) return;

    int nextIndex = currentTrackIndex + 1;
    if (nextIndex >= trackCount) nextIndex = 0;

    Serial.println("MP3SongList: Track ended, auto-advancing");
    playTrack(nextIndex);
    updateNowPlaying();
}

void MP3SongList::handleTouch(int x, int y) {
    extern MP3Player mp3Player;

    if (backButton.hit(x, y)) {
        Serial.println("MP3SongList: Back pressed");
        mp3Player.requestStop();
        screenManager.showAlbumList();
        return;
    }

    if (prevButton.hit(x, y)) {
        if (trackCount > 0) {
            int newIndex = currentTrackIndex - 1;
            if (newIndex < 0) newIndex = trackCount - 1;
            playTrack(newIndex);
            updateNowPlaying();
        }
        return;
    }

    if (playPauseButton.hit(x, y)) {
        isPlaying = !isPlaying;
        if (isPlaying) {
            mp3Player.resume();
            playPauseButton.setLabel("Pause");
        } else {
            mp3Player.pause();
            playPauseButton.setLabel("Play");
        }
        playPauseButton.draw(tft);
        return;
    }

    if (nextButton.hit(x, y)) {
        if (trackCount > 0) {
            int newIndex = currentTrackIndex + 1;
            if (newIndex >= trackCount) newIndex = 0;
            playTrack(newIndex);
            updateNowPlaying();
        }
        return;
    }

    if (volumeSlider.handleTouch(x, y)) {
        int volume = volumeSlider.getValue();
        audioModule.setVolume(volume);
        volumeSlider.draw(tft);
        return;
    }

    if (maxScrollOffset > 0 && trackScrollSlider.handleTouch(x, y)) {
        updateScrollOffsetFromSlider();

        const int SNAP_MARGIN = 15;
        if (y <= TRACK_Y_START + SNAP_MARGIN) {
            scrollOffset = 0;
        } else if (y >= TRACK_Y_START + TRACK_AREA_H - SNAP_MARGIN) {
            scrollOffset = maxScrollOffset;
        }

        drawTrackListArea();
        return;
    }

    // Tap a specific track row to jump straight to it.
    if (x >= TRACK_X - 5 && x < TRACK_TAP_RIGHT_X &&
        y >= TRACK_Y_START && y < TRACK_Y_START + TRACK_AREA_H) {
        int rowIndex = (y - TRACK_Y_START) / TRACK_ROW_H;
        int trackIndex = scrollOffset + rowIndex;

        if (trackIndex < trackCount) {
            Serial.printf("MP3SongList: Row tapped -> '%s'\n", trackNames[trackIndex]);
            playTrack(trackIndex);
            updateNowPlaying();
        }
        return;
    }
}
