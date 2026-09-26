#include <TinyGPSPlus.h>

#define GPS_UART_RX 18
#define GPS_UART_TX 17

HardwareSerial GPS(1);
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  delay(1000);

  GPS.begin(38400, SERIAL_8N1, GPS_UART_RX, GPS_UART_TX);

  Serial.println("FIELDDECK GPS TEST");
}

void loop() {

  while (GPS.available()) {
    gps.encode(GPS.read());
  }

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 1000) {
    lastPrint = millis();

    Serial.println();
    Serial.println("----- GPS DATA -----");

    if (gps.location.isValid()) {
      Serial.print("Latitude:  ");
      Serial.println(gps.location.lat(), 6);

      Serial.print("Longitude: ");
      Serial.println(gps.location.lng(), 6);
    } else {
      Serial.println("Location: WAITING FOR FIX");
    }

    if (gps.altitude.isValid()) {
      Serial.print("Altitude:  ");
      Serial.print(gps.altitude.feet());
      Serial.println(" ft");
    } else {
      Serial.println("Altitude:  WAITING");
    }

    if (gps.satellites.isValid()) {
      Serial.print("Satellites: ");
      Serial.println(gps.satellites.value());
    } else {
      Serial.println("Satellites: WAITING");
    }

    if (gps.hdop.isValid()) {
      Serial.print("HDOP:       ");
      Serial.println(gps.hdop.hdop());
    } else {
      Serial.println("HDOP:       WAITING");
    }

    if (gps.speed.isValid()) {
      Serial.print("Speed:      ");
      Serial.print(gps.speed.mph());
      Serial.println(" mph");
    }

    Serial.println("--------------------");
  }
}