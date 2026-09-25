#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

// I2C pins
#define SDA_PIN 15
#define SCL_PIN 16

// Create BNO055 sensor
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Start I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("Starting BNO055...");

  if (!bno.begin()) {
    Serial.println("ERROR: BNO055 not found!");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("BNO055 connected!");

  // Give the sensor time to initialize
  delay(1000);

  // Use external crystal for better accuracy if your module has one
  bno.setExtCrystalUse(true);

  Serial.println("BNO055 ready!");
  Serial.println();
}

void loop() {
  // Get orientation
  sensors_event_t orientationData;
  bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);

  Serial.print("Heading: ");
  Serial.print(orientationData.orientation.x);
  Serial.println("°");

  Serial.print("Roll: ");
  Serial.print(orientationData.orientation.y);
  Serial.println("°");

  Serial.print("Pitch: ");
  Serial.print(orientationData.orientation.z);
  Serial.println("°");

  Serial.println("--------------------");

  delay(500);
}