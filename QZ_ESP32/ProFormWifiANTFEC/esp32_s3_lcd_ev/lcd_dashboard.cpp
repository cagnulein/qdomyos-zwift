#include "lcd_dashboard.h"

#include <Arduino.h>
#include <ctype.h>
#include <esp_heap_caps.h>
#include <esp_display_panel.hpp>

namespace {

constexpr int kWidth = 480;
constexpr int kHeight = 480;
constexpr size_t kPixelCount = kWidth * kHeight;
constexpr uint32_t kBackground = 0x081220;
constexpr uint32_t kHeader = 0x11243C;
constexpr uint32_t kCard = 0x11243C;
constexpr uint32_t kWhite = 0xFFFFFF;
constexpr uint32_t kMuted = 0x9AAEC4;
constexpr uint32_t kGreen = 0x36D399;
constexpr uint32_t kOrange = 0xF5A524;
constexpr uint32_t kRed = 0xF87272;
constexpr uint32_t kBlue = 0x4D9FFF;

// Five columns per glyph, with the low bit at the top of each column.
// Character order: digits, uppercase letters, space, dash, period, colon, slash, question mark, plus.
const uint8_t kFont[][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
    {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E},
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, {0x7F, 0x49, 0x49, 0x49, 0x36},
    {0x3E, 0x41, 0x41, 0x41, 0x22}, {0x7F, 0x41, 0x41, 0x22, 0x1C},
    {0x7F, 0x49, 0x49, 0x49, 0x41}, {0x7F, 0x09, 0x09, 0x09, 0x01},
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, {0x7F, 0x08, 0x08, 0x08, 0x7F},
    {0x00, 0x41, 0x7F, 0x41, 0x00}, {0x20, 0x40, 0x41, 0x3F, 0x01},
    {0x7F, 0x08, 0x14, 0x22, 0x41}, {0x7F, 0x40, 0x40, 0x40, 0x40},
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, {0x7F, 0x04, 0x08, 0x10, 0x7F},
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, {0x7F, 0x09, 0x09, 0x09, 0x06},
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, {0x7F, 0x09, 0x19, 0x29, 0x46},
    {0x46, 0x49, 0x49, 0x49, 0x31}, {0x01, 0x01, 0x7F, 0x01, 0x01},
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, {0x1F, 0x20, 0x40, 0x20, 0x1F},
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, {0x63, 0x14, 0x08, 0x14, 0x63},
    {0x07, 0x08, 0x70, 0x08, 0x07}, {0x61, 0x51, 0x49, 0x45, 0x43},
    {0x00, 0x00, 0x00, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08},
    {0x00, 0x60, 0x60, 0x00, 0x00}, {0x00, 0x36, 0x36, 0x00, 0x00},
    {0x20, 0x10, 0x08, 0x04, 0x02}, {0x02, 0x01, 0x51, 0x09, 0x06},
    {0x08, 0x08, 0x3E, 0x08, 0x08},
};

esp_panel::board::Board *board = nullptr;
esp_panel::drivers::LCD *lcd = nullptr;
esp_panel::drivers::Touch *touch = nullptr;
uint8_t *pixels = nullptr;
bool touchWasDown = false;

uint32_t rgb888(uint8_t red, uint8_t green, uint8_t blue) {
  return (static_cast<uint32_t>(red) << 16) |
         (static_cast<uint32_t>(green) << 8) | blue;
}

const uint8_t *glyphFor(char value) {
  const char ch = static_cast<char>(toupper(static_cast<unsigned char>(value)));
  if (ch >= '0' && ch <= '9') {
    return kFont[ch - '0'];
  }
  if (ch >= 'A' && ch <= 'Z') {
    return kFont[10 + ch - 'A'];
  }
  switch (ch) {
    case ' ': return kFont[36];
    case '-': return kFont[37];
    case '.': return kFont[38];
    case ':': return kFont[39];
    case '/': return kFont[40];
    case '+': return kFont[42];
    default: return kFont[41];
  }
}

void fillRect(int x, int y, int width, int height, uint32_t color) {
  const int x0 = max(0, x);
  const int y0 = max(0, y);
  const int x1 = min(kWidth, x + width);
  const int y1 = min(kHeight, y + height);
  for (int row = y0; row < y1; ++row) {
    for (int col = x0; col < x1; ++col) {
      const uint8_t red = static_cast<uint8_t>((color >> 16) & 0xFF);
      const uint8_t green = static_cast<uint8_t>((color >> 8) & 0xFF);
      const uint8_t blue = static_cast<uint8_t>(color & 0xFF);
      const uint16_t rgb565 = static_cast<uint16_t>(
          ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3));
      const size_t offset = (static_cast<size_t>(row) * kWidth + col) * 2;
      pixels[offset] = static_cast<uint8_t>(rgb565 & 0xFF);
      pixels[offset + 1] = static_cast<uint8_t>(rgb565 >> 8);
    }
  }
}

void drawText(int x, int y, int scale, uint32_t color, const char *text) {
  int cursor = x;
  for (const char *ch = text; *ch != '\0'; ++ch) {
    const uint8_t *glyph = glyphFor(*ch);
    for (int col = 0; col < 5; ++col) {
      for (int row = 0; row < 7; ++row) {
        if ((glyph[col] >> row) & 1) {
          fillRect(cursor + col * scale, y + row * scale, scale, scale, color);
        }
      }
    }
    cursor += 6 * scale;
  }
}

void centerText(int x, int y, int width, int scale, uint32_t color, const char *text) {
  const int textWidth = static_cast<int>(strlen(text)) * 6 * scale - scale;
  drawText(x + max(0, (width - textWidth) / 2), y, scale, color, text);
}

void drawBadge(int x, const char *title, const char *state, uint32_t color) {
  fillRect(x, 70, 145, 64, kCard);
  fillRect(x, 70, 4, 64, color);
  drawText(x + 10, 80, 1, kMuted, title);
  drawText(x + 10, 105, 1, color, state);
}

void drawMetricCard(int x, const char *title, const char *value, const char *unit,
                    uint32_t accent) {
  fillRect(x, 148, 145, 286, kCard);
  fillRect(x, 148, 145, 5, accent);
  centerText(x, 174, 145, 1, kMuted, title);
  centerText(x, 246, 145, 5, kWhite, value);
  centerText(x, 350, 145, 1, accent, unit);
}

void drawTargetButton(int x, const char *label, uint32_t color) {
  fillRect(x, 375, 60, 52, color);
  centerText(x, 389, 60, 3, kWhite, label);
}

}  // namespace

bool lcdDashboardBegin() {
  pixels = static_cast<uint8_t *>(heap_caps_malloc(
      kPixelCount * 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (pixels == nullptr) {
    pixels = static_cast<uint8_t *>(malloc(kPixelCount * 2));
  }
  if (pixels == nullptr) {
    Serial.println("[LCD] Unable to allocate the dashboard framebuffer");
    return false;
  }

  board = new esp_panel::board::Board();
  board->init();
  const bool boardReady = board->begin();
  if (!boardReady) {
    // Board::begin() also initializes the touch controller. The dashboard does
    // not use touch, so keep going if the LCD itself reached BEGIN successfully.
    Serial.println("[LCD] Board initialization incomplete; checking LCD state");
  }

  lcd = board->getLCD();
  touch = board->getTouch();
  if (lcd == nullptr || !lcd->isOverState(esp_panel::drivers::LCD::State::BEGIN) ||
      lcd->getFrameWidth() != kWidth || lcd->getFrameHeight() != kHeight ||
      lcd->getFrameColorBits() != 16) {
    Serial.println("[LCD] LCD did not reach 480x480 RGB565 ready state");
    return false;
  }

  auto *backlight = board->getBacklight();
  if (backlight != nullptr) {
    backlight->on();
  }

  Serial.println("[LCD] 3.95-inch 480x480 dashboard ready");
  return true;
}

void lcdDashboardUpdate(const LcdDashboardData &data) {
  if (lcd == nullptr || pixels == nullptr) {
    return;
  }

  fillRect(0, 0, kWidth, kHeight, kBackground);
  fillRect(0, 0, kWidth, 56, kHeader);
  drawText(18, 13, 3, kWhite, "QZ PROFORM BRIDGE");

  drawBadge(10, "WIFI", data.wifiConnected ? "CONNESSO" : "OFFLINE",
            data.wifiConnected ? kGreen : kRed);
  drawBadge(167, "BICI / WS", data.bikeConnected ? "CONNESSA" : "OFFLINE",
            data.bikeConnected ? kGreen : kRed);

  const char *antState = "XIAO OFFLINE";
  uint32_t antColor = kRed;
  if (data.xiaoOnline && !data.antChannelReady) {
    antState = "ANT+ ERRORE";
    antColor = kRed;
  } else if (data.xiaoOnline && data.fecPeerActive) {
    antState = "FE-C ATTIVO";
    antColor = kGreen;
  } else if (data.xiaoOnline) {
    antState = "PRONTO / ATTESA";
    antColor = kOrange;
  }
  drawBadge(324, "ANT+ FE-C", antState, antColor);

  char watts[16] = "---";
  char cadence[16] = "---";
  char target[16] = "---";
  if (data.telemetryFresh) {
    snprintf(watts, sizeof(watts), "%.0f", data.watts);
    snprintf(cadence, sizeof(cadence), "%.0f", data.cadenceRpm);
  }
  if (data.requestedWatts > 0.0f) {
    snprintf(target, sizeof(target), "%.0f", data.requestedWatts);
  }

  drawMetricCard(10, "WATT ATTUALI", watts, "W", kBlue);
  drawMetricCard(167, "CADENZA", cadence, "RPM", rgb888(189, 96, 255));
  drawMetricCard(324, "WATT RICHIESTI", target, "ERG / FE-C", kOrange);
  drawTargetButton(334, "-", kBlue);
  drawTargetButton(400, "+", kGreen);
  centerText(0, 453, kWidth, 1, kMuted,
             "I DATI SONO LETTI DALLA BICI PROFORM SULLA RETE WI-FI");

  if (!lcd->drawBitmap(0, 0, kWidth, kHeight, pixels, -1)) {
    Serial.println("[LCD] Dashboard frame write failed");
  }
}

int lcdDashboardReadErgAdjustment() {
  if (touch == nullptr || !touch->isOverState(esp_panel::drivers::Touch::State::BEGIN)) {
    return 0;
  }

  esp_panel::drivers::TouchPoint point;
  const int points = touch->readPoints(&point, 1, 0);
  if (points <= 0) {
    touchWasDown = false;
    return 0;
  }
  if (touchWasDown) {
    return 0;
  }
  touchWasDown = true;

  if (point.y < 375 || point.y >= 427) {
    return 0;
  }
  if (point.x >= 334 && point.x < 394) {
    return -1;
  }
  if (point.x >= 400 && point.x < 460) {
    return 1;
  }
  return 0;
}
