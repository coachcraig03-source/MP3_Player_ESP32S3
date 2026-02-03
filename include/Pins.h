// =====================================================================
//  ESP32-S3 NFC Music Player - Pin Definitions
//  Hardware: ESP32-S3-DevKitC-1-N8R8
//  Components: ILI9341 TFT + Touch, PN5180 NFC, VS1053B Audio, SDMMC
// =====================================================================

#ifndef PINS_H
#define PINS_H

// =========================
//   SHARED SPI BUS (HSPI)
//   All SPI devices share these pins
// =========================
#define SPI_SCK         12      // Shared clock
#define SPI_MOSI        11      // Shared MOSI
#define SPI_MISO        13      // Shared MISO

// =========================
//   TFT DISPLAY (ILI9341)
//   Uses shared SPI bus
// =========================
#define TFT_CS          10      // TFT Chip Select
#define TFT_DC          9       // TFT Data/Command
#define TFT_RST         -1      // TFT Reset (not connected)
#define TFT_BL          21      // TFT Backlight (PWM capable for dimming)

// =========================
//   TOUCH CONTROLLER (XPT2046)
//   Uses shared SPI bus
// =========================
#define TOUCH_CS        38      // Touch Chip Select
#define TOUCH_IRQ       39      // Touch Interrupt (optional, for efficiency)

// =========================
//   PN5180 NFC READER
//   Uses shared SPI bus
// =========================
#define NFC_CS          16       // PN5180 Chip Select (NSS)
#define NFC_RST         17      // PN5180 Reset
#define NFC_BUSY        14      // PN5180 Busy/IRQ signal
#define NFC_AUX         -1      // PN5180 AUX (optional, for advanced features)
#define NFC_IRQ         15

// =========================
//   VS1053B AUDIO DECODER
//   Uses shared SPI bus
// =========================
#define VS1053_XCS     -1      // VS1053 Command Chip Select
#define VS1053_XDCS     7       // VS1053 Data Chip Select
#define VS1053_DREQ     -1     // VS1053 Data Request (MUST be interrupt-capable)
#define VS1053_RST      -1      // VS1053 Reset

// =========================
//   SDMMC (4-bit mode)
//   Separate bus from SPI
// =========================
#define SDMMC_CLK       36      // SD Card Clock
#define SDMMC_CMD       35      // SD Card Command
#define SDMMC_D0        37      // SD Card Data 0
#define SDMMC_D1        34      // SD Card Data 1
#define SDMMC_D2        33      // SD Card Data 2
#define SDMMC_D3        4       // SD Card Data 3

// =========================
//   USER INTERFACE
// =========================
#define LED_STATUS      48      // Built-in RGB LED on DevKit
#define BUTTON_MODE     0       // Boot button (built-in)

#endif // PINS_H