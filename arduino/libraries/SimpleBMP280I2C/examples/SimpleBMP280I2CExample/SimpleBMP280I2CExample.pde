#include "Arduino.h"
#include <I2C.h> // https://github.com/rambo/I2C
#include <SimpleBMP280I2C.h>

/* This is an example for the BMP280 barometric pressure sensor
 * The operation mode mode is set to to:
 * t_sb = 101 = 1000 ms
 * filter = 000 = off
 * osrs_t = 001 = 1
 * osrs_p = 001 = 1
 * mode = 11 = normal mode
 */

SimpleBMP280I2C baro; // for I2C address 0x76

// SimpleBMP280I2C baro(0x77); // for I2C address 0x77


void setup() {
  Serial.begin(9600);
  I2c.begin();
  I2c.setSpeed(1);
  I2c.timeOut(10);

  if (!baro.begin()) {
	Serial.println(F("No BMP280 sensor found!"));
	while (1) {}
  }
}

void loop() {
	Serial.print(F("Pressure = "));
	Serial.print(baro.getPressure(), DEC);
	Serial.println(F(" Pa"));

	Serial.print(F("Temperature = "));
	Serial.print(baro.getLastTemperature(), DEC);
	Serial.println(F(" cC"));

	Serial.println();
	delay(1000);
}
