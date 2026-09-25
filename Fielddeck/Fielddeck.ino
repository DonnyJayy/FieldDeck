#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SDA_PIN 15
#define SCL_PIN 16

Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  // Try the two common BME280 I2C addresses
  if (!bme.begin(0x76, &Wire)) {
    Serial.println("BME280 not found at 0x76, trying 0x77...");

    if (!bme.begin(0x77, &Wire)) {
      Serial.println("ERROR: BME280 not found!");
      while (1) {
        delay(1000);
      }
    }
  }

  Serial.println("BME280 connected!");
  Serial.println();
}

void loop() {
  Serial.print("Temperature: ");
  Serial.print((bme.readTemperature() * 9.0 / 5.0) + 32.0);
  Serial.println(" °F");

  Serial.print("Humidity: ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.println("--------------------");

  delay(2000);
}