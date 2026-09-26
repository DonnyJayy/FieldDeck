#include <Wire.h>
#include <math.h>

#define MAG_ADDR 0x2C

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(15, 16);

  Serial.println("QMC5883P COMPASS TEST");

  // Configure QMC5883P
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x0A);
  Wire.write(0x0D);
  Wire.endTransmission();

  delay(100);
}

void loop() {

  // Read status register
  Wire.beginTransmission(MAG_ADDR);
  Wire.write(0x09);
  Wire.endTransmission(false);

  Wire.requestFrom(MAG_ADDR, 1);

  if (Wire.available()) {

    byte status = Wire.read();

    if (status & 0x01) {

      // Read X/Y/Z
      Wire.beginTransmission(MAG_ADDR);
      Wire.write(0x01);
      Wire.endTransmission(false);

      Wire.requestFrom(MAG_ADDR, 6);

      if (Wire.available() >= 6) {

        int16_t x = Wire.read();
        x |= Wire.read() << 8;

        int16_t y = Wire.read();
        y |= Wire.read() << 8;

        int16_t z = Wire.read();
        z |= Wire.read() << 8;

        // Calculate heading
        float heading = atan2((float)y, (float)x) * 180.0 / PI;

        // Convert negative angles to 0-360
        if (heading < 0) {
          heading += 360.0;
        }

        Serial.print("X: ");
        Serial.print(x);

        Serial.print("  Y: ");
        Serial.print(y);

        Serial.print("  Z: ");
        Serial.print(z);

        Serial.print("  Heading: ");
        Serial.print(heading, 1);

        Serial.println("°");
      }
    }
  }

  delay(100);
}