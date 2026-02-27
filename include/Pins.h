// =====================================================================
//  ESP32-S3 NFC Music Player - Pin Definitions
//  Hardware: ESP32-S3-DevKitC-1-N8R8
// =====================================================================

#ifndef PINS_H
#define PINS_H

// =========================
//   SPI BUS 1 (VSPI) - Audio & NFC
//   PN5180 NFC + VS1053B Audio Decoder
// =========================
#define SPI1_SCK         12      // Shared clock
#define SPI1_MOSI        11      // Shared MOSI
#define SPI1_MISO        13      // Shared MISO

// =========================
//   RC522 NFC READER
//   ** WORKING - DO NOT CHANGE **
// =========================
#define NFC_CS          16      // RC522 Chip Select (NSS)
#define NFC_RST         18      // RC522 Reset

// =========================
//   VS1053B AUDIO DECODER
//   Uses SPI1 bus (11, 12, 13)
// =========================
#define VS1053_CS       9       // VS1053 SPI Chip Select
#define VS1053_DCS      7       // VS1053 Data Chip Select (XDCS)
#define VS1053_DREQ     10//2       // VS1053 Data Request (must be interrupt-capable)
#define VS1053_RST      8       // VS1053 Reset (XRST)

// =========================
//   SPI BUS 2 (HSPI) - Display & SD Card
//   4.0" ST7796S TFT with SD card
// =========================
#define SPI2_SCK        36      // HSPI Clock (pin 7)
#define SPI2_MOSI       35      // HSPI MOSI (pin 6 - SDI)
#define SPI2_MISO       47//37      // HSPI MISO (pin 9 - SDO)

// =========================
//   TFT DISPLAY (ST7796S 4.0")
//   320x480 resolution
// =========================
#define TFT_CS          38      // LCD Chip Select (pin 3) - GPIO 33 not exposed
#define TFT_RST         39      // LCD Reset (pin 4) - GPIO 34 not exposed
#define TFT_DC          40      // LCD Data/Command (pin 5 - LCD_RS)
#define TFT_BL          41      // LCD Backlight (pin 8 - LED)


// =========================
//   SD CARD 
// =========================
#define SD_CS           2       // SDMMC Data 3

// Bluetooth Control
#define BT_ENABLE_PIN   1       // VHM-314 power control


// =========================
//   CAPACITIVE TOUCH (I2C)
//   Separate I2C bus - no conflicts
// =========================
#define I2C_SDA         5//45      // Touch IIC Data (pin 12 - CTP_SDA)
#define I2C_SCL         6//46      // Touch IIC Clock (pin 10 - CTP_SCL)
#define TOUCH_RST       4//37//47      // Touch Reset (pin 11 - CTP_RST)
#define TOUCH_INT       48      // Touch Interrupt (pin 13 - CTP_INT)

// =========================
//   USER INTERFACE
// =========================
//#define LED_STATUS      21      // Built-in RGB LED (GPIO 48 used for touch)
#define BUTTON_MODE     0       // Boot button (built-in)

// =========================
//   NOTES
// =========================
// SPI1 (11,12,13): PN5180 NFC + VS1053B Audio
// SPI2 (35,36,37): ST7796S TFT + SD Card
// I2C  (45,46):    Capacitive Touch
//
// GPIO 33 & 34 are NOT exposed on ESP32-S3-DevKitC-1
// Adjusted all pins to available GPIOs (38-42, 45-48)

#endif // PINS_H