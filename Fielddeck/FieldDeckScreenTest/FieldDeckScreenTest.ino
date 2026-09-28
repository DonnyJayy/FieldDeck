#include <GxEPD2_BW.h>
#include <other/GxEPD2_420_SE0420NQ04.h>
#include <SPI.h>
#include <Wire.h>
#include <TinyGPSPlus.h>

// E-PAPER
#define EPD_CS    45
#define EPD_DC    46
#define EPD_RST   47
#define EPD_BUSY  48
#define EPD_SCK   12
#define EPD_MOSI  11
#define PWR_MAIN  41
#define PWR_EPD   7

// GPS
#define GPS_UART_RX 18
#define GPS_UART_TX 17
HardwareSerial GPS(1);
TinyGPSPlus gps;

// COMPASS
#define SDA_PIN 15
#define SCL_PIN 16
#define QMC5883P_ADDR 0x2C

// DISPLAY
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

unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 5000;

// COMPASS READING
float heading = 0.0;

void readCompass() {

  int16_t x, y, z;

  Wire.beginTransmission(QMC5883P_ADDR);
  Wire.write(0x01);
  Wire.endTransmission();

  Wire.requestFrom(QMC5883P_ADDR, 6);

  if (Wire.available() >= 6) {

    x = Wire.read();
    x |= Wire.read() << 8;

    y = Wire.read();
    y |= Wire.read() << 8;

    z = Wire.read();
    z |= Wire.read() << 8;

    heading = atan2((float)y, (float)x) * 180.0 / PI;

    if (heading < 0) {
      heading += 360.0;
    }
  }
}

// COMPASS DIRECTION
const char* getDirection(float degrees) {

  if (degrees >= 337.5 || degrees < 22.5)
    return "N";

  if (degrees >= 22.5 && degrees < 67.5)
    return "NE";

  if (degrees >= 67.5 && degrees < 112.5)
    return "E";

  if (degrees >= 112.5 && degrees < 157.5)
    return "SE";

  if (degrees >= 157.5 && degrees < 202.5)
    return "S";

  if (degrees >= 202.5 && degrees < 247.5)
    return "SW";

  if (degrees >= 247.5 && degrees < 292.5)
    return "W";

  return "NW";
}

// GPS QUALITY
const char* getGPSQuality() {

  if (!gps.location.isValid()) {
    return "SEARCHING";
  }

  if (!gps.hdop.isValid()) {
    return "UNKNOWN";
  }

  double hdop = gps.hdop.hdop();

  if (hdop < 1.0) {
    return "EXCELLENT";
  }

  if (hdop < 2.0) {
    return "STRONG";
  }

  if (hdop < 5.0) {
    return "GOOD";
  }

  return "WEAK";
}

// UPDATE DISPLAY
void updateDisplay() {

  display.setFullWindow();

  display.firstPage();

  do {

    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);

    // TITLE
    display.setTextSize(2);
    display.setCursor(20, 35);
    display.print("FIELDDECK");

    display.drawLine(
      20, 45,
      380, 45,
      GxEPD_BLACK
    );

    // GPS
    display.setTextSize(1);

    display.setCursor(20, 70);
    display.print("LAT: ");

    if (gps.location.isValid()) {
      display.print(gps.location.lat(), 6);
    } else {
      display.print("SEARCHING...");
    }

    display.setCursor(20, 95);
    display.print("LON: ");

    if (gps.location.isValid()) {
      display.print(gps.location.lng(), 6);
    } else {
      display.print("SEARCHING...");
    }

    display.setCursor(20, 120);
    display.print("ALT: ");

    if (gps.altitude.isValid()) {
      display.print(gps.altitude.feet(), 0);
      display.print(" FT");
    } else {
      display.print("---");
    }

    // GPS QUALITY
    display.setCursor(20, 150);
    display.print("GPS: ");
    display.print(getGPSQuality());

    // COMPASS
    display.setCursor(20, 185);
    display.print("HEADING: ");

    display.print(heading, 1);
    display.print((char)247);

    display.print("  ");
    display.print(getDirection(heading));

    // COMPASS RAW
    display.setCursor(20, 215);
    display.print("COMPASS: OK");

    // GPS FIX
    display.setCursor(20, 245);

    if (gps.location.isValid()) {
      display.print("GPS FIX: YES");
    } else {
      display.print("GPS FIX: SEARCHING");
    }

  } while (display.nextPage());
}

// SETUP
void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("FIELDDECK GPS + COMPASS");
  Serial.println("==============================");

  // DISPLAY POWER
  pinMode(PWR_MAIN, OUTPUT);
  digitalWrite(PWR_MAIN, HIGH);

  pinMode(PWR_EPD, OUTPUT);
  digitalWrite(PWR_EPD, HIGH);

  delay(500);

  // E-PAPER SPI
  SPI.begin(
    EPD_SCK,
    -1,
    EPD_MOSI,
    EPD_CS
  );

  display.init(
    115200,
    true,
    2,
    false
  );

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

  // COMPASS
  Wire.begin(
    SDA_PIN,
    SCL_PIN
  );

  Wire.beginTransmission(QMC5883P_ADDR);

  byte error = Wire.endTransmission();

  if (error == 0) {

    Serial.println(
      "QMC5883P FOUND at 0x2C"
    );

    // Reset/configure compass
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x0A);
    Wire.write(0x80);
    Wire.endTransmission();

    delay(10);

    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x0A);
    Wire.write(0x0D);
    Wire.endTransmission();

  } else {

    Serial.println(
      "QMC5883P NOT FOUND"
    );
  }

  Serial.println("SYSTEM READY");
}

// LOOP
void loop() {

  // Continuously read GPS
  while (GPS.available()) {
    gps.encode(GPS.read());
  }

  // Continuously read compass
  readCompass();

  // SERIAL DEBUG
  static unsigned long lastSerial = 0;

  if (millis() - lastSerial >= 2000) {

    lastSerial = millis();

    Serial.println();
    Serial.println("----- FIELDDECK STATUS -----");

    if (gps.location.isValid()) {

      Serial.print("Latitude:  ");
      Serial.println(
        gps.location.lat(),
        6
      );

      Serial.print("Longitude: ");
      Serial.println(
        gps.location.lng(),
        6
      );

      Serial.print("Altitude:  ");
      Serial.print(
        gps.altitude.feet(),
        0
      );
      Serial.println(" ft");

      Serial.print("HDOP:      ");
      Serial.println(
        gps.hdop.hdop(),
        2
      );

      Serial.print("GPS:       ");
      Serial.println(
        getGPSQuality()
      );

    } else {

      Serial.println(
        "GPS: SEARCHING"
      );
    }

    Serial.print("Heading:   ");
    Serial.print(
      heading,
      1
    );

    Serial.print(" deg (");
    Serial.print(
      getDirection(heading)
    );
    Serial.println(")");
  }

  // DISPLAY UPDATE
  if (
    millis() - lastDisplayUpdate >=
    DISPLAY_INTERVAL
  ) {

    lastDisplayUpdate = millis();

    updateDisplay();
  }
}