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

#define FILE "HidnSeek_v3_28"
#include "EEPROM.h"
#include "LowPower.h"
#include "HidnSeek.h"
#include "TinyGPS.h"
#include "def.h"

HidnSeek HidnSeek(txSigfox, rxSigfox);

TinyGPS gps;

#include "MMA8653.h"
MMA8653 accel;

#include "Barometer.h"
Barometer bmp180;

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

void initGPIO()
{
  // Set output to 0 for not used Pads
  PORTB = (DIGITAL_PULLUP >> 8) & 0xff;
  DDRB  = (DIGITAL_OUTPUT >> 8) & 0xff;
  PORTC = 0x00;
  DDRC  = DDRC_MASK;
  PORTD = DIGITAL_PULLUP & 0xff;
  DDRD  = DIGITAL_OUTPUT & 0xff;
}

void flashRed(int num) {
  while (num > 0) {
    PORTD |= (1 << redLEDpin);
    delay(50);
    PORTD &= ~(1 << redLEDpin);
    if (--num) delay(50);
  }
}

void NoflashRed() {
  delay(25);
  PORTD &= ~(1 << redLEDpin) & ~(forceSport << bluLEDpin);
  delay(50);
  PORTD |= (1 << redLEDpin) | (forceSport << bluLEDpin);
}

int powerDownLoop(int msgs) {
  if (batterySense()) shutdownSys(); // else digitalWrite(shdPin, HIGH);
  if (forceSport) {
    if ((batteryPercent < 25) || (limitSport++ >= SPORT_LIMIT)) {
      //serialString(PSTR("Sport Limit")); Serial.println();
      forceSport = 0;
    }
  } else gpsStandby();

  if (syncSat < 255) {
    sendSigFox(msgs); // if not arround previous location send new position
  }

  accelStatus(); // record the current angle
  if (msgs == MSG_POSITION && spd > 15 && noSat == 0) detectMotion = MOTION_MIN_NUMBER << 1; else detectMotion = 0;

  // Loop duration 8s. 75x 10mn, 150x 20mn,
  static uint8_t countNoMotion;
  if (msgs != MSG_NO_MOTION) countNoMotion = 0;

  uint8_t modeSport = forceSport;

  unsigned int waitLoop;
  if (msgs == MSG_NO_MOTION) {
    waitLoop = 420 << countNoMotion; // 1h loop
    hour += (1 << countNoMotion);
    if (hour > 23) {
      hour = 0;
      day++;
    }
    if (day > 31) {
      day = 0;
      month++;
    }
    if (month > 12) {
      month = 0;
      year++;
    }
    if (countNoMotion < 3) countNoMotion++;
  } else {
    waitLoop = (PERIOD_COUNT >> forceSport) - loopGPS;  // 10mn loop: 6mn sleep + 4mn for GPS
  }

  unsigned int i = 0;

  period_t sleepDuration;
  if (msgs != MSG_NO_MOTION && detectMotion == 0 && forceSport == 0) sleepDuration = SLEEP_4S; else sleepDuration = SLEEP_8S;

  Serial.flush();
  while (i < waitLoop) {
    LowPower.powerDown(sleepDuration, ADC_OFF, BOD_OFF);
    PORTD |= (1 << redLEDpin) | (forceSport << bluLEDpin);
    if (GPSactive) {
      batterySense();
      if (batteryPercent < 98 && !forceSport) gpsStandby();
    }
    if (accelStatus()) { // device moved
      if (GPSactive) NoflashRed(); else delay(50);
      detectMotion++;
      if (sleepDuration == SLEEP_4S) {
        sleepDuration = SLEEP_8S;
        i = i >> 1;
      }
      if (msgs == MSG_NO_MOTION || modeSport != forceSport) waitLoop = 0; // exit immediatly or stay in 5mn loop
    }
    i++;
    PORTD &= ~(1 << redLEDpin) & ~(1 << bluLEDpin);
  }
  detectMotion = (detectMotion > MOTION_MIN_NUMBER || forceSport) ? 1 : 0;
  if (msgs == MSG_NO_MOTION && waitLoop == 0) detectMotion = -1; // This mean a motion after a while
  if (detectMotion > 0 && !GPSactive) GPSactive = gpsInit();
  start = millis();
  loopGPS = syncSat = noSat = 0;
  alt = spd = 0;
  p.lat = p.lon = 0;
  return detectMotion;
}

int main(void)
{
  init();
  delay(100);

  initGPIO();

  Serial.begin(9600);
  Serial.println();
  serialString(PSTR("Firmware " FILE "\nRelease " __DATE__ " " __TIME__));
  Serial.println();

  dumpEEprom();

  initSense();
  batterySense();
  serialString(PSTR(" Battery: "));
  Serial.print(batteryPercent);
  serialString(PSTR("% "));
  Serial.println(batteryValue);
  delay(100);
  if (GPSactive = gpsInit()) {
    gpsCmd(PSTR(PMTK_VERSION));
    flashRed(1);
  }
  if (accelPresent = initMems()) {
    delay(500);
    if (accelStatus()) flashRed(2);
  }

  if (baromPresent = bmp180.init()) {
    delay(500);
    bmp180Measure(&Temp, &Press);
    flashRed(3);
  }
  bmp180Print();

  if (initSigFox()) {
    delay(500);
    flashRed(4);
  } else {
    PORTD |= (1 << bluLEDpin);
    Serial.flush();
    delay(500);
    digitalWrite(shdPin, LOW);
    delay(1000);
  }

  serialString(PSTR("free Ram: "));
  Serial.println(freeRam());

  // Change charge current to 250mA Not recommanded if connected to a computer, use with wall adapter only.
  //digitalWrite(chg500mA, HIGH);

  start = millis();

  while (1) {

    if ((uint16_t) (millis() - start) >= 4000) {
      blueLEDon;
      delay(100);
      accelStatus();
      blueLEDoff;
      loopGPS++;
      start = millis();
    }

    // if a sentence is received, we can check the checksum, parse it...
    if (detectMotion == 1) {
      if (gpsProcess()) LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
    }

    // Let 2mn to acquire GPS position otherwise go to sleep until accelerometer wake up.
    if ( syncSat >= 30 && noSat == 0) {
      detectMotion = powerDownLoop(MSG_POSITION);
    }

    if ( loopGPS > 16 || noSat > 120) {
      detectMotion = powerDownLoop(MSG_NO_GPS);
    }

    if (detectMotion == 0) detectMotion = powerDownLoop(MSG_NO_MOTION);
    if (detectMotion == -1) detectMotion = powerDownLoop(MSG_MOTION_ALERT); // Alert for Motion detected after the blank time
  }
}
