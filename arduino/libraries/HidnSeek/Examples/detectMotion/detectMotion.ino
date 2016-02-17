/* detectMotion

 Turns on and off a light emitting diode (LED) according to motion.
 If motion RED LED, if not BLUE LED blink

 created 2016 by Stephane Driussi
*/

#define txSigfox         5     // PD5 TX Serial to Sigfox modem
#define bluLEDpin        6     // PD6 Piezzo Output
#define redLEDpin        7     // PD7 Red LED Status
#define rxSigfox         8     // PB0 RX Serial from Sigfox modem
#define ACCEL_MODE       2
#define SENSITIVITY      2

#include "SoftwareSerial.h"
#include "HidnSeek.h"
HidnSeek HidnSeek(txSigfox, rxSigfox);

#include "Wire.h"
#include "MMA8653.h"
MMA8653 accel;

// Variables will change :
int ledState = LOW;             // ledState used to set the LED
int ledColor = redLEDpin;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long start;

void setup() {
  HidnSeek.initGPIO(false);
  HidnSeek.setSupply(true);
  Wire.begin();
  accel.begin(false, ACCEL_MODE);
  start = millis();
}

bool accelStatus() {
  static int8_t x, y, z;

  boolean accelMove = false;
  byte error = accel.update();

  if (error != 0) accelMove = true;
  else {
    accelMove = ((uint8_t)(abs((int8_t)(accel.getX() - x))) > SENSITIVITY) ? true :
                ((uint8_t)(abs((int8_t)(accel.getY() - y))) > SENSITIVITY) ? true :
                ((uint8_t)(abs((int8_t)(accel.getZ() - z))) > SENSITIVITY) ? true : false;
    x = accel.getX();
    y = accel.getY();
    z = accel.getZ();
  }
  return accelMove;
}

void loop()
{
  HidnSeek.checkBattery();

  unsigned long currentMillis = millis();

  if ((currentMillis - start) > 30000) HidnSeek.setSupply(false);

  if (currentMillis - previousMillis >= 200) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (accelStatus())      ledColor = redLEDpin;      else ledColor = bluLEDpin;
    digitalWrite(ledColor, HIGH);
    delay(50);
    digitalWrite(ledColor, LOW);
  }
}

