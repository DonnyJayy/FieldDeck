#include <GxEPD2_BW.h>
#include <other/GxEPD2_420_SE0420NQ04.h>
#include <SPI.h>
#include <TinyGPSPlus.h>

// CrowPanel ESP32-S3 E-Paper 4.2" V1.2

// E-Paper pins
#define EPD_CS    45
#define EPD_DC    46
#define EPD_RST   47
#define EPD_BUSY  48

// E-Paper SPI pins
#define EPD_SCK   12
#define EPD_MOSI  11

// Display power
#define PWR_MAIN  41
#define PWR_EPD   7

// GPS

#define GPS_UART_RX 18
#define GPS_UART_TX 17

HardwareSerial GPS(1);
TinyGPSPlus gps;

// Display object

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

// Timing

unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 5000;

// Setup

void setup() {

  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("FIELDDECK GPS + DISPLAY TEST");
  Serial.println("==============================");

  // Display power

  pinMode(PWR_MAIN, OUTPUT);
  digitalWrite(PWR_MAIN, HIGH);

  pinMode(PWR_EPD, OUTPUT);
  digitalWrite(PWR_EPD, HIGH);

  delay(500);

  Serial.println("Display power ON");

  // Display SPI

  SPI.begin(
    EPD_SCK,
    -1,
    EPD_MOSI,
    EPD_CS
  );

  Serial.println("Display SPI started");

  // Initialize display

  display.init(
    115200,
    true,
    2,
    false
  );

  Serial.println("Display initialized");

  display.epd2.selectSPI(
    SPI,
    SPISettings(
      4000000,
      MSBFIRST,
      SPI_MODE0
    )
  );

  // GPS

  GPS.begin(
    38400,
    SERIAL_8N1,
    GPS_UART_RX,
    GPS_UART_TX
  );

  Serial.println("GPS UART started");
  Serial.println("Waiting for GPS data...");
}

// Update display


void updateDisplay() {

  Serial.println();
  Serial.println("Updating display...");

  display.setFullWindow();

  display.firstPage();

  do {

    display.fillScreen(GxEPD_WHITE);

    display.setTextColor(GxEPD_BLACK);

    // Title
    display.setTextSize(2);
    display.setCursor(20, 35);
    display.print("FIELDDECK GPS");

    // GPS data
    display.setTextSize(1);

    display.setCursor(20, 75);
    display.print("LAT: ");

    if (gps.location.isValid()) {
      display.print(gps.location.lat(), 6);
    } else {
      display.print("SEARCHING...");
    }

    display.setCursor(20, 100);
    display.print("LON: ");

    if (gps.location.isValid()) {
      display.print(gps.location.lng(), 6);
    } else {
      display.print("SEARCHING...");
    }

    display.setCursor(20, 130);
    display.print("ALT: ");

    if (gps.altitude.isValid()) {
      display.print(gps.altitude.feet(), 0);
      display.print(" FT");
    } else {
      display.print("---");
    }

    display.setCursor(20, 215);
    display.print("SPEED: ");

    if (gps.speed.isValid()) {
      display.print(gps.speed.mph(), 1);
      display.print(" MPH");
    } else {
      display.print("---");
    }

    // Status
    display.setCursor(20, 260);

    if (gps.location.isValid()) {
      display.print("GPS FIX: YES");
    } else {
      display.print("GPS FIX: SEARCHING");
    }

  } while (display.nextPage());

  Serial.println("Display update complete");
}

void loop() {

  // Read GPS data
  while (GPS.available()) {
    gps.encode(GPS.read());
  }

  // Print GPS data when available
  static unsigned long lastSerial = 0;

  if (millis() - lastSerial >= 2000) {

    lastSerial = millis();

    Serial.println();
    Serial.println("----- GPS STATUS -----");

    if (gps.location.isValid()) {

      Serial.print("Latitude:  ");
      Serial.println(gps.location.lat(), 6);

      Serial.print("Longitude: ");
      Serial.println(gps.location.lng(), 6);

      Serial.print("Altitude:  ");
      Serial.print(gps.altitude.feet(), 0);
      Serial.println(" ft");

    } else {

      Serial.println("Waiting for GPS fix...");

    }
  }

  // Update e-paper every 5 seconds
  if (millis() - lastDisplayUpdate >= DISPLAY_INTERVAL) {

    lastDisplayUpdate = millis();

    updateDisplay();
  }
}