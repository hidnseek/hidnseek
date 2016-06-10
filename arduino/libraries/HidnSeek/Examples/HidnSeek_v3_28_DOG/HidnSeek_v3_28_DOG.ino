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

#define FILE "HidnSeek v3.28 DOG"
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

void pin2_isr()
{
  button_pressed = true;
}

void pin3_isr()
{
  button_pressed = true;
}

void initButton(uint8_t button) {
  if (button == accINT) {
    attachInterrupt(1, pin3_isr, FALLING);
  } else {
    attachInterrupt(0, pin2_isr, FALLING);
  }
  button_pressed = false;
  button_flag = 0;

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
    delay(32);
    PORTD &= ~(1 << redLEDpin);
    if (--num) delay(64);
  }
}

void NoflashRed() {
  delay(32);
  PORTD &= ~(1 << redLEDpin) & ~(forceSport << bluLEDpin);
  delay(64);
  PORTD |= (1 << redLEDpin) | (forceSport << bluLEDpin);
}

int powerDownLoop(int msgs) {
  if (batterySense()) shutdownSys(); // else digitalWrite(shdPin, HIGH);
  if ((batteryPercent < 25)) {
    forceSport = 0;
  }
  gpsStandby();

  sendSigFox(msgs); // if not arround house location send new position

  accelStatus(); // record the current angle
  if (msgs == MSG_POSITION && spd > 5 && noSat == 0) {
    detectMotion = MOTION_MIN_NUMBER << 1;
  } else {
    detectMotion = 0;
  }

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
    waitLoop = forceSport ? 410 : PERIOD_COUNT - loopGPS;  // 10mn loop: 6mn sleep + 4mn for GPS
  }

  unsigned int i = 0;
  button_pressed = false;
  button_flag = 0;

  period_t sleepDuration;
  if (msgs != MSG_NO_MOTION && detectMotion == 0 && forceSport == 0) sleepDuration = SLEEP_4S; else sleepDuration = SLEEP_8S;

  Serial.flush();
  while (i < waitLoop) {
    LowPower.powerDown(sleepDuration, ADC_OFF, BOD_OFF);
    if (button_pressed || forceSport) {
      PORTD |= (1 << bluLEDpin);
      if (button_pressed) waitLoop = 0;
    }
    PORTD |= (1 << redLEDpin);
    if (accelStatus()) { // device moved
      delay(50);
      detectMotion++;
      if (sleepDuration == SLEEP_4S) {
        sleepDuration = SLEEP_8S;
        i = i >> 1;
      }
      if  (msgs == MSG_NO_MOTION || modeSport != forceSport) waitLoop = 0; // exit immediatly or stay in 5mn loop
    }
    if ( (detectMotion >= MOTION_MIN_NUMBER) && (i > DOG_COUNT) && forceSport)  waitLoop = 0; // exit immediatly or stay in 5mn loop
    i++;
    PORTD &= ~(1 << redLEDpin) & ~(1 << bluLEDpin);
  }
  if (detectMotion >= MOTION_MIN_NUMBER || forceSport || button_pressed) {
    detectMotion = 1;
  } else if (msgs == MSG_NO_MOTION && i > waitLoop) {
    // This mean a motion after a while
    detectMotion = -1;
  }
  else {
    detectMotion = 0;
  }
  if (detectMotion > 0) {
    GPSactive = gpsInit();
  }
  start = millis();
  loopGPS = syncSat = noSat = sat = 0;
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
    bmp180Print();
    flashRed(3);
  } else {
    Temp = 20;
    Press = 1030;
    serialString(PSTR("Temp fail\r\n"));
  }

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

  initButton(accINT);

  start = millis();

  while (1) {

    if ((uint16_t) (millis() - start) >= 4000) {
      PORTD |= (1 << bluLEDpin);
      delay(100);
      accelStatus();
      PORTD &= ~(1 << bluLEDpin);
      loopGPS++;
      start = millis();
    }

    // if a sentence is received, we can check the checksum, parse it...
    if (detectMotion == 1) {
      if (gpsProcess()) LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
    }

    // Let 2mn to acquire GPS position otherwise go to sleep until accelerometer wake up.
    if ((( syncSat >= 64 && (spd < 11 || spd > 80)) || syncSat >= 128) && noSat == 0) {
      detectMotion = powerDownLoop(MSG_POSITION);
    }

    if ( loopGPS > 30 || noSat > 120) {
      detectMotion = powerDownLoop(MSG_NO_GPS);
    }

    if (button_pressed && button_flag == 0) {
      button_pressed = false;
      alt = spd = 0;
      p.lat = p.lon = 0;
      sendSigFox(MSG_OPTION);
      button_flag = 1;
    }

    if (detectMotion == 0) detectMotion = powerDownLoop(MSG_NO_MOTION);
    if (detectMotion == -1) detectMotion = powerDownLoop(MSG_MOTION_ALERT); // Alert for Motion detected after the blank time
  }
}
