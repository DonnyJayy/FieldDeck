#include <GxEPD2_BW.h>
#include <other/GxEPD2_420_SE0420NQ04.h>
#include <SPI.h>

// ========================================
// CrowPanel ESP32-S3 E-Paper 4.2" V1.2
// ========================================

// E-Paper pins
#define EPD_CS    45
#define EPD_DC    46
#define EPD_RST   47
#define EPD_BUSY  48

// SPI pins
#define EPD_SCK   12
#define EPD_MOSI  11

// Display power rails
#define PWR_MAIN  41
#define PWR_EPD   7

// ========================================
// Display object
// ========================================

GxEPD2_BW<
  GxEPD2_420_SE0420NQ04,
  GxEPD2_420_SE0420NQ04::HEIGHT
> display(
  GxEPD2_420_SE0420NQ04(
    EPD_CS,
    EPD_DC,
    EPD_RST,
    EPD_BUSY
  )
);

// ========================================
// Setup
// ========================================

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("FIELDDECK V1.2 DISPLAY TEST");
  Serial.println("==============================");

  // Turn on the display power rails
  pinMode(PWR_MAIN, OUTPUT);
  digitalWrite(PWR_MAIN, HIGH);

  pinMode(PWR_EPD, OUTPUT);
  digitalWrite(PWR_EPD, HIGH);

  delay(500);

  Serial.println("Power rails ON");

  // Start SPI using the CrowPanel V1.2 pins
  SPI.begin(
    EPD_SCK,
    -1,
    EPD_MOSI,
    EPD_CS
  );

  Serial.println("SPI started");

  // Initialize the e-paper
  display.init(
    115200,
    true,
    2,
    false
  );

  Serial.println("Display initialized");

  // Select the SPI settings
  display.epd2.selectSPI(
    SPI,
    SPISettings(
      4000000,
      MSBFIRST,
      SPI_MODE0
    )
  );

  Serial.println("SPI settings applied");

  // Full-screen update
  display.setFullWindow();

  Serial.println("Drawing test screen...");

  display.firstPage();

  do {

    display.fillScreen(GxEPD_WHITE);

    display.setTextColor(GxEPD_BLACK);

    display.setTextSize(2);

    display.setCursor(30, 60);
    display.print("FIELDDECK");

    display.setCursor(30, 110);
    display.print("V1.2 DISPLAY TEST");

    display.setCursor(30, 160);
    display.print("HELLO WORLD");

    display.setCursor(30, 210);
    display.print("400 x 300");

  } while (display.nextPage());

  Serial.println("DISPLAY UPDATE COMPLETE");

  // Put display to sleep
  display.hibernate();

  Serial.println("DISPLAY SLEEP");
}

// ========================================
// Main loop
// ========================================

void loop() {
}