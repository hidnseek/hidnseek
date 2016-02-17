/* Blink without Delay

 Turns on and off a light emitting diode (LED) connected to a digital
 pin, without using the delay() function.  This means that other code
 can run at the same time without being interrupted by the LED code.

 The circuit:
 * LED attached from pin 13 to ground.
 * Note: on most Arduinos, there is already an LED on the board
 that's attached to pin 13, so no hardware is needed for this example.

 created 2005
 by David A. Mellis
 modified 8 Feb 2010
 by Paul Stoffregen
 modified 11 Nov 2013
 by Scott Fitzgerald
 modified 16 Feb 2016
 by Stephane Driussi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/BlinkWithoutDelay
 */

#define txSigfox         5     // PD5 TX Serial to Sigfox modem
#define bluLEDpin        6     // PD6 Piezzo Output
#define redLEDpin        7     // PD7 Red LED Status
#define rxSigfox         8     // PB0 RX Serial from Sigfox modem

#include "SoftwareSerial.h"
#include "HidnSeek.h"
HidnSeek HidnSeek(txSigfox, rxSigfox);

// Variables will change :
int ledState = LOW;             // ledState used to set the LED
int ledColor = redLEDpin;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated
unsigned long interval = 1000;           // interval at which to blink (milliseconds)
unsigned long start;

void setup() {
  HidnSeek.initGPIO(false);
  HidnSeek.setSupply(true);
  start = millis();
}

void loop()
{
  HidnSeek.checkBattery();
  
  unsigned long currentMillis = millis();

  // here is where you'd put code that needs to be running all the time.
  if ((currentMillis - start) > 30000) HidnSeek.setSupply(false);

  // check to see if it's time to blink the LED; that is, if the
  // difference between the current time and last time you blinked
  // the LED is bigger than the interval at which you want to
  // blink the LED.

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
      if (ledColor == redLEDpin) ledColor = bluLEDpin; else ledColor = redLEDpin; 
      interval = 100;
    } else {
      ledState = LOW;
      interval = 400;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(ledColor, ledState);
  }
}

