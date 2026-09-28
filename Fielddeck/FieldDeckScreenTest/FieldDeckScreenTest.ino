#include <GxEPD2_BW.h>
#include <other/GxEPD2_420_SE0420NQ04.h>
#include <SPI.h>
#include <Wire.h>
#include <TinyGPSPlus.h>
#include <Adafruit_BME280.h>

// E-PAPER V1.2
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

// QMC5883P COMPASS
#define COMPASS_SDA 15
#define COMPASS_SCL 16
#define QMC5883P_ADDR 0x2C

// BME280
#define BME_SDA 20
#define BME_SCL 19
Adafruit_BME280 bme;
bool bmeFound = false;
bool compassFound = false;

// I2C BUSES
// Bus 0 = GPS board compass
TwoWire CompassWire = Wire;

// Bus 1 = BME280
TwoWire BMEWire = TwoWire(1);

// E-PAPER OBJECT
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

// TIMING
unsigned long lastDisplayUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 5000;

// COMPASS HEADING
float heading = 0.0;

// READ QMC5883P - Compass
void readCompass() {

  int16_t x, y, z;

  CompassWire.beginTransmission(QMC5883P_ADDR);
  CompassWire.write(0x01);
  CompassWire.endTransmission();

  CompassWire.requestFrom(QMC5883P_ADDR, 6);

  if (CompassWire.available() >= 6) {

    x = CompassWire.read();
    x |= CompassWire.read() << 8;

    y = CompassWire.read();
    y |= CompassWire.read() << 8;

    z = CompassWire.read();
    z |= CompassWire.read() << 8;

    heading =
      atan2((float)y, (float)x)
      * 180.0 / PI;

    if (heading < 0) {
      heading += 360.0;
    }
  }
}

// GET COMPASS DIRECTION
const char* getDirection(float degrees) {

  if (degrees >= 337.5 || degrees < 22.5)
    return "N";

  if (degrees < 67.5)
    return "NE";

  if (degrees < 112.5)
    return "E";

  if (degrees < 157.5)
    return "SE";

  if (degrees < 202.5)
    return "S";

  if (degrees < 247.5)
    return "SW";

  if (degrees < 292.5)
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

  if (hdop < 1.0)
    return "EXCELLENT";

  if (hdop < 2.0)
    return "STRONG";

  if (hdop < 5.0)
    return "GOOD";

  return "WEAK";
}

// DISPLAY
void updateDisplay() {

  display.setFullWindow();

  display.firstPage();

  do {

    display.fillScreen(GxEPD_WHITE);

    display.setTextColor(GxEPD_BLACK);

    // HEADER
    display.setTextSize(2);

    display.setCursor(20, 30);

    display.print("FIELDDECK");

    display.drawLine(
      20,
      40,
      380,
      40,
      GxEPD_BLACK
    );

    // LOCATION
    display.setTextSize(1);

    display.setCursor(20, 65);

    display.print("LAT: ");

    if (gps.location.isValid()) {

      display.print(
        gps.location.lat(),
        6
      );

    } else {

      display.print("SEARCHING...");
    }

    display.setCursor(20, 85);

    display.print("LON: ");

    if (gps.location.isValid()) {

      display.print(
        gps.location.lng(),
        6
      );

    } else {

      display.print("SEARCHING...");
    }

    display.setCursor(20, 105);

    display.print("ALT: ");

    if (gps.altitude.isValid()) {

      display.print(
        gps.altitude.feet(),
        0
      );

      display.print(" FT");

    } else {

      display.print("---");
    }

    // GPS QUALITY
    display.setCursor(20, 130);

    display.print("GPS: ");

    display.print(
      getGPSQuality()
    );

    // COMPASS
    display.setCursor(20, 160);

    display.print("HEADING: ");

    display.print(
      heading,
      1
    );

    display.print((char)247);

    display.print(" ");

    display.print(
      getDirection(heading)
    );

    // WEATHER / ENVIRONMENT
    display.setCursor(20, 190);

    display.print("TEMP: ");

    if (bmeFound) {

      float tempF =
        bme.readTemperature()
        * 9.0 / 5.0
        + 32.0;

      display.print(
        tempF,
        1
      );

      display.print(" F");

    } else {

      display.print("---");
    }

    display.setCursor(20, 210);

    display.print("HUMIDITY: ");

    if (bmeFound) {

      display.print(
        bme.readHumidity(),
        1
      );

      display.print(" %");

    } else {

      display.print("---");
    }

    display.setCursor(20, 230);

    display.print("PRESSURE: ");

    if (bmeFound) {

      display.print(
        bme.readPressure() / 100.0F,
        1
      );

      display.print(" hPa");

    } else {

      display.print("---");
    }

    // SYSTEM STATUS
    display.setCursor(20, 260);

    if (gps.location.isValid()) {

      display.print("GPS: OK");

    } else {

      display.print("GPS: SEARCHING");
    }

    display.setCursor(180, 260);

    if (compassFound) {

      display.print("COMPASS: OK");

    } else {

      display.print("COMPASS: ERROR");
    }

    display.setCursor(300, 260);

    if (bmeFound) {

      display.print("BME: OK");

    } else {

      display.print("BME: ERROR");
    }

  } while (display.nextPage());
}

// SETUP
void setup() {

  Serial.begin(115200);

  delay(2000);

  Serial.println();
  Serial.println("==============================");
  Serial.println("FIELDDECK SENSOR INTEGRATION");
  Serial.println("==============================");

  // E-PAPER POWER
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

  Serial.println("E-paper initialized.");

  // GPS
  GPS.begin(
    38400,
    SERIAL_8N1,
    GPS_UART_RX,
    GPS_UART_TX
  );

  Serial.println("GPS initialized.");

  // COMPASS I2C
  CompassWire.begin(
    COMPASS_SDA,
    COMPASS_SCL
  );

  CompassWire.beginTransmission(
    QMC5883P_ADDR
  );

  byte compassError =
    CompassWire.endTransmission();

  if (compassError == 0) {

    compassFound = true;

    Serial.println(
      "QMC5883P FOUND at 0x2C"
    );

    // Reset
    CompassWire.beginTransmission(
      QMC5883P_ADDR
    );

    CompassWire.write(0x0A);
    CompassWire.write(0x80);

    CompassWire.endTransmission();

    delay(10);

    // Continuous mode
    CompassWire.beginTransmission(
      QMC5883P_ADDR
    );

    CompassWire.write(0x0A);
    CompassWire.write(0x0D);

    CompassWire.endTransmission();

  } else {

    Serial.println(
      "QMC5883P NOT FOUND"
    );
  }

  // BME280 I2C
  BMEWire.begin(
    BME_SDA,
    BME_SCL
  );

  Serial.println(
    "BME280 I2C started."
  );

  if (bme.begin(0x76, &BMEWire)) {

    bmeFound = true;

    Serial.println(
      "BME280 FOUND at 0x76"
    );

  } else if (bme.begin(0x77, &BMEWire)) {

    bmeFound = true;

    Serial.println(
      "BME280 FOUND at 0x77"
    );

  } else {

    Serial.println(
      "BME280 NOT FOUND"
    );
  }

  Serial.println();
  Serial.println("==============================");
  Serial.println("FIELDDECK READY");
  Serial.println("==============================");
}

// LOOP
void loop() {

  // GPS
  while (GPS.available()) {

    gps.encode(
      GPS.read()
    );
  }

  // COMPASS
  readCompass();

  // SERIAL STATUS
  static unsigned long lastSerial = 0;

  if (millis() - lastSerial >= 2000) {

    lastSerial = millis();

    Serial.println();
    Serial.println(
      "----- FIELDDECK STATUS -----"
    );

    // GPS
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

    // Compass
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

    // BME280
    if (bmeFound) {

      float tempF =
        bme.readTemperature()
        * 9.0 / 5.0
        + 32.0;

      Serial.print("Temp:      ");
      Serial.print(
        tempF,
        1
      );

      Serial.println(" F");

      Serial.print("Humidity:  ");
      Serial.print(
        bme.readHumidity(),
        1
      );

      Serial.println(" %");

      Serial.print("Pressure:  ");
      Serial.print(
        bme.readPressure() / 100.0F,
        1
      );

      Serial.println(" hPa");
    }
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