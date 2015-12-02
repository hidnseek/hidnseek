/*  This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 th  e Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 HidnSeek by StephaneD 20150313 Not for commercial use              */

#include "avr/pgmspace.h"
#include "SoftwareSerial.h"
#include "LowPower.h"
#include "Akeru.h"
#include "TinyGPS.h"
#include "def.h"

Akeru SigFox(txSigfox, rxSigfox);

TinyGPS gps;

#include "Wire.h"
#include "MMA8653.h"
MMA8653 accel;

#include <SFE_BMP180.h>
SFE_BMP180 pressure;

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void serialString (PGM_P s) {

  char c;
  while ((c = pgm_read_byte(s++)) != 0)
    Serial.print(c);
}

// TODO Maintenance message checkout on start
// TODO Request downlink the configuration after the dance on reboot
// TODO Shake to send immediate alert

void initGPIO()
{
  // Set output to 0 for not used Pads
  pinMode(sensorA0, OUTPUT);
  digitalWrite(sensorA0, LOW);

  pinMode(piezzo, OUTPUT);
  digitalWrite(piezzo, LOW);

  pinMode(chg500mA, OUTPUT);
  digitalWrite(chg500mA, LOW);

  pinMode(redLEDpin, OUTPUT);
  digitalWrite(redLEDpin, LOW);

  pinMode(rstPin, OUTPUT);
  digitalWrite(rstPin, LOW);

  pinMode(satLEDpin, INPUT);

  pinMode(accINT, INPUT);
  digitalWrite(accINT, HIGH);

  pinMode(usbDM, INPUT);
  digitalWrite(usbDM, HIGH); // Pull-up already done by external 1.5K

  pinMode(usbDP, INPUT);
  digitalWrite(usbDP, LOW);

  digitalWrite(shdPin, HIGH);
  pinMode(shdPin, OUTPUT);

  pinMode(piezzo, OUTPUT);
  digitalWrite(piezzo, LOW);

  for (uint8_t i = 11; i < 14; i++) { // SPI bus on pull-up
    digitalWrite (i, HIGH);
    pinMode (i, INPUT);
  }
}

void flashRed(int num) {
  while (num > 0) {
    digitalWrite(redLEDpin, HIGH);
    delay(50);
    digitalWrite(redLEDpin, LOW);
    delay(50);
    num = num - 1;
  }
}

void flashRed() {
  digitalWrite(redLEDpin, HIGH);
  delay(25);
  digitalWrite(redLEDpin, LOW);
  delay(50);
}

void NoflashRed() {
  delay(25);
  digitalWrite(redLEDpin, LOW);
  delay(50);
  digitalWrite(redLEDpin, HIGH);
}

int powerDownLoop(int msgs) {
  if (batterySense()) shutdownSys(); else digitalWrite(shdPin, HIGH);
  //if (batteryValue < min(985, batteryCharge) || (msgs == MSG_NO_MOTION)) gpsStandby();
  if (batteryPercent < 95) gpsStandby(); else gpsInit();
  batteryCharge = batteryValue + 5;  // Add 20mV hysterisis
  if (syncSat < 255) sendSigFox(msgs); // if not arround previous location send new position
  syncSat = 0;
  unsigned int i = 0;
  detectMotion = 0;
  accelStatus(); // record the current position
  // Loop duration 8s. 75x 10mn, 150x 20mn,
  static uint8_t countNoMotion;
  if (msgs != MSG_NO_MOTION) countNoMotion = 0;
  unsigned int waitLoop;
  switch (msgs) {
    case MSG_NO_GPS:  // 20mn loop
      waitLoop = 38; // 150;
      break;
    case MSG_NO_MOTION:
      waitLoop = 420 << countNoMotion; // 450=1h loop, ajusted to 420 on 24/11/15
      if (++countNoMotion > 3) countNoMotion = 3;
      break;
    default:
      waitLoop = 38;  // 5mn loop
      break;
  }
  Serial.flush();
  while (i < waitLoop) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    digitalWrite(redLEDpin, HIGH);
    if (GPSactive) {
      batterySense();
      if (batteryPercent < 95) gpsStandby();
    }
    if (accelStatus()) { // device moved
      if (GPSactive) NoflashRed(); else delay(50);
      detectMotion++;
      waitLoop = (msgs == MSG_NO_MOTION) ? 0 : 38;                         // exit immediatly or stay in 5mn loop
    }
    if (i == 38 && detectMotion == 0 && msgs != MSG_NO_MOTION) i = waitLoop;  // Exit and enter in no motion mode
    i++;
    digitalWrite(redLEDpin, LOW);
  }
  detectMotion = (detectMotion > MOTION_MIN_NUMBER) ? 1 : 0;
  if (msgs == MSG_NO_MOTION && i > waitLoop) detectMotion = -1; // This mean a motion after a while
  if (detectMotion > 0) GPSactive = gpsInit();
  start = millis();
  loopCW = millis();
  noSat = 0;
  return detectMotion;
}

int main(void)
{
  init();
  delay(100);

  initGPIO();

  Serial.begin(9600);
  Serial.println();
  serialString(PSTR("Firmware " __FILE__ "\nRelease " __DATE__ " " __TIME__));
  Serial.println();
  serialString(PSTR("TinyGPS lib v. "));
  Serial.println(TinyGPS::library_version());

  dumpEEprom();

  initSense();
  batterySense();
  serialString(PSTR("Init batterie reg: "));
  Serial.println(batteryValue);
  batteryCharge = batteryValue + 1;  // Add 4mV hysterisis
  delay(100);
  if (GPSactive = gpsInit()) {
    gpsCmd(PSTR(PMTK_VERSION));
    flashRed(1);
  }
  if (accelPresent = initMems()) {
    delay(500);
    if (accelStatus()) flashRed(2);
  }

  if (baromPresent = pressure.begin()) {
    delay(500);
    bmp180Measure(&Temp, &Press);
    bmp180Print();
    flashRed(3);
  } else {
    serialString(PSTR("Temp fail"));
    Serial.println();
  }

  if (modemPresent = initSigFox()) {
    delay(500);
    flashRed(4);
  } else {
    Serial.flush();
    delay(100);

    digitalWrite(shdPin, LOW);
    delay(100);
  }

  serialString(PSTR("free Ram: "));
  Serial.println(freeRam());

  while (1) {
    // when millis() or start timer wraps around after low power, we'll just reset it
    if (start > millis())  start = millis();

    if (millis() - loopCW > 8000) {
      digitalWrite(bluLEDpin, HIGH);
      delay(100);
      accelStatus();
      digitalWrite(bluLEDpin, LOW);
      loopCW = millis();
    }

    // if a sentence is received, we can check the checksum, parse it...
    if (detectMotion == 1 && gpsProcess()) {
      if (syncSat > 30) {
        detectMotion = powerDownLoop(MSG_POSITION);
      }
    }

    // Let 4mn to acquire GPS position otherwise go to sleep until accelerometer wake up.
    // Reduce the delay to 2mn if no satellites are visible
    if ((millis() - start > 240000) || noSat > 120) {
      detectMotion = powerDownLoop(noSat == 0 ? MSG_POSITION : MSG_NO_GPS);
    }

    if (detectMotion == 0) detectMotion = powerDownLoop(MSG_NO_MOTION);
    if (detectMotion == -1) detectMotion = powerDownLoop(MSG_MOTION_ALERT); // Alert for Motion detected after the blank time
  }
}
