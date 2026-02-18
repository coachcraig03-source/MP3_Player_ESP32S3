// =====================================================================
//  TFT_Module.cpp - ST7796S Display Module Implementation
// =====================================================================

#include "TFT_Module.h"
#include "pins.h"

// Define LGFX class with ST7796 configuration
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7796 _panel_instance;
  lgfx::Bus_SPI _bus_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();

      cfg.spi_host = SPI3_HOST;  // HSPI on ESP32-S3
      //cfg.spi_host = SPI_HOST_MAX;   // Forces software SPI (GPIO-matrix)

      cfg.spi_mode = 0;
      cfg.freq_write = 40000000;  // 40MHz
      cfg.freq_read = 16000000;
      cfg.spi_3wire = false;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = SPI2_SCK;
      cfg.pin_mosi = SPI2_MOSI;
      cfg.pin_miso = SPI2_MISO;
      cfg.pin_dc = TFT_DC;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs = TFT_CS;
      cfg.pin_rst = TFT_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 320;
      cfg.panel_height = 480;
      cfg.offset_x = 0;
      cfg.offset_y = 0;
      _panel_instance.config(cfg);
    }
    setPanel(&_panel_instance);
  }
};

TFT_Module::TFT_Module(uint8_t cs, uint8_t dc, uint8_t rst, uint8_t bl,
                       uint8_t sck, uint8_t mosi, uint8_t miso)
  : _cs(cs), _dc(dc), _rst(rst), _bl(bl),
    _sck(sck), _mosi(mosi), _miso(miso), tft(nullptr)
{
}

// At the top of TFT_Module.cpp, make hspi global to this file
static SPIClass hspi(HSPI);

bool TFT_Module::begin() {
  Serial.println("TFT: Initializing ST7796S...");
  
  // Initialize HSPI separately from global SPI
  //hspi.begin(SPI2_SCK, SPI2_MISO, SPI2_MOSI);
  Serial.println("TFT: HSPI initialized separately");
  
  // Setup backlight
  if (_bl >= 0) {
    pinMode(_bl, OUTPUT);
    digitalWrite(_bl, HIGH);
    Serial.println("TFT: Backlight ON");
  }
  
  // Create LGFX object (it will use SPI2_HOST internally)
  Serial.println("TFT: Creating display object...");
  tft = new LGFX();
  
  Serial.println("TFT: Calling init()...");
  tft->init();
  // Restore SPI1 after TFT init 
  //SPI.begin(SPI1_SCK, SPI1_MISO, SPI1_MOSI);
  
  // DON'T touch global SPI here
  
  tft->setRotation(3);
  Serial.printf("TFT: Display size: %d x %d\n", tft->width(), tft->height());
  tft->fillScreen(TFT_BLACK);
  
  Serial.println("TFT: Ready!");
  return true;
}

lgfx::LGFX_Device* TFT_Module::getTFT() {
  return tft;
}

void TFT_Module::setBacklight(bool on) {
  if (_bl >= 0) {
    digitalWrite(_bl, on ? HIGH : LOW);
  }
}

void TFT_Module::setRotation(uint8_t rotation) {
  if (tft) {
    tft->setRotation(rotation);
  }
}