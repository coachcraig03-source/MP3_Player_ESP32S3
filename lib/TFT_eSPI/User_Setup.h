// =====================================================================
//  User_Setup.h for TFT_eSPI
//  ESP32-S3 NFC Music Player - ILI9341 Display
// =====================================================================

#define USER_SETUP_ID 200

// Driver
#define ILI9341_DRIVER

// Display size
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// Pin definitions - must match pins.h
#define TFT_MISO 13
#define TFT_MOSI 11
#define TFT_SCLK 12
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  -1  // Not connected

// Fonts
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// SPI frequency - start conservative
#define SPI_FREQUENCY  10000000   // 10MHz
#define SPI_READ_FREQUENCY 10000000