/**************************************************************************
  TTGO T-Display ST7789 OLED based on Adafruit example
  https://github.com/adafruit/Adafruit-ST7735-Library/blob/master/examples/graphicstest/graphicstest.ino
 **************************************************************************/
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#include <DHT.h>
#define DHTPIN 26
#define DHTTYPE DHT22
#define DHTVCCPIN 32

// pinouts from https://github.com/Xinyuan-LilyGO/TTGO-T-Display
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TFT_BL 4

SPIClass tftSPI = SPIClass(VSPI);
Adafruit_ST7789 tft = Adafruit_ST7789(&tftSPI, TFT_CS, TFT_DC, TFT_RST);
bool wasnan = true;
bool firstboot = true;
float last_temp, last_hum;

void setup(void) {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);      // TTGO T-Display enable Backlight pin 4
  digitalWrite(TFT_BL, HIGH);   // T-Display turn on Backlight
  tftSPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
  tft.init(135, 240);           // Initialize ST7789 240x135
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setSPISpeed(10000000);

  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, LOW);
  Serial.println(F("Initialized"));
}

void dht_read(float * h_ptr, float * t_ptr) {
  Serial.println("Reading from DHT22 sensor");
  tft.drawRect(0, 0, 240, 135, ST77XX_CYAN);
  pinMode(DHTVCCPIN, OUTPUT);
  digitalWrite(DHTVCCPIN, HIGH);
  delay(125);
  DHT dht(DHTPIN, DHTTYPE);
  dht.begin();
  delay(125);
  *h_ptr = dht.readHumidity();
  *t_ptr = dht.readTemperature();
  digitalWrite(DHTVCCPIN, LOW);
  tft.drawRect(0, 0, 240, 135, ST77XX_BLACK);
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, LOW);
  Serial.printf("Temperature: %.1f, Humidity: %.1f\r\n", *t_ptr, *h_ptr);
  Serial.println("Finished reading");
}

void drawValues(float t, float h, bool black) {
  tft.setCursor(20, 20);
  tft.setTextWrap(true);
  tft.setTextSize(5);
  if (black) {
    tft.setTextColor(ST77XX_BLACK);
  } else if (t < 20) {
    tft.setTextColor(ST77XX_BLUE);
  } else if (t < 22) {
    tft.setTextColor(ST77XX_GREEN);
  } else if (t < 24) {
    tft.setTextColor(ST77XX_YELLOW);
  } else if (t < 26) {
    tft.setTextColor(ST77XX_ORANGE);
  } else if (t < 28) {
    tft.setTextColor(ST77XX_RED);
  } else {
    tft.setTextColor(ST77XX_MAGENTA);
  }
  tft.print(h, 1);
  tft.println(" %");
  tft.setCursor(20, tft.getCursorY());
  tft.print(t, 1);
  tft.println(" C");
}

void loop() {
  if (firstboot) {
    firstboot = false;
  } else {
    delay(9750);
  }

  float h, t;

  dht_read(&h, &t);

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT22 sensor!"));
    if (!wasnan) {
      tft.fillScreen(ST77XX_RED);
      wasnan = true;
      last_temp = t;
      last_hum = h;
    }
    return;
  } else {
    if (wasnan) {
      tft.fillScreen(ST77XX_BLACK);
    }
    wasnan = false;
  }

  if (last_temp != t || last_hum != h) {
    drawValues(last_temp, last_hum, true);
    drawValues(t, h, false);
    last_temp = t;
    last_hum = h;
  }
}
