/*  This file is part of HidnSeek.

  HidnSeek is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  HidnSeek is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with HidnSeek.  If not, see <http://www.gnu.org/licenses/>.*/

#define ACCEL_MODE 2        // 2G scale for the accelerometer
#define ACCEL_TRIG 40
#define ACCEL_FLAT 10
#define SPORT_LIMIT 48      // 4h limit sport duration
#define PERIOD_LOOP 10      // number of minutes between messages, must be > 10
#define MOTION_MIN_NUMBER 2

// --------------- do not change parameters below this line -------------------

#define PERIOD_COUNT ((PERIOD_LOOP * 15) >> 1)

#define BATT_MIN 3570
#define BATT_MAX 4200

// EEPROM map
#define ADDR_TODAY     0
#define ADDR_SENT      1 // byte01-31: number of messages sent
#define ADDR_CAL_LOW  32 // byte32-33: battery calibration
#define ADDR_CAL_HIGH 33

#define PMTK_SET_NMEA_OUTPUT  "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*" // 28"

#define PMTK_AWAKE   "$PMTK010,001*" // 2E"
#define PMTK_STANDBY "$PMTK161,0*"   // 28"
#define PMTK_VERSION "$PMTK605*"     // 31"
#define PMTK_SET_NMEA_UPDATE_1HZ "$PMTK220,1000*" // 1F"
#define PMTK_ENABLE_SBAS "$PMTK313,1*" // 2E"
#define PMTK_ENABLE_WAAS "$PMTK301,2*" // 2E"

enum msgs {
  MSG_POSITION = 0, MSG_OPTION = 3, MSG_NO_MOTION = 4, MSG_NO_GPS = 5,
  MSG_MOTION_ALERT = 6, MSG_WEAK_BAT = 7
};

enum {
  POS_FACE_UP = 0, POS_FACE_DN = 1, POS_SIDE_UP = 2,
  POS_SIDE_DN = 3, POS_SIDE_RT = 4, POS_SIDE_LT = 5,
  POS_NULL = 6
};

/****************** Pins usage ***************************************/
#define rxGPS            0     // PD0 RX Serial from GPS
#define txGPS            1     // PD1 TX Serial to GPS
#define usbDP            2     // PD2 Shutdown supply after power off
#define accINT           3     // PB0 Accelerometer Interruption
#define usbDM            4     // PD4
#define txSigfox         5     // PD5 TX Serial to Sigfox modem
#define piezzo           6     // PD6 Piezzo Output
#define bluLEDpin        6     // PD6 Piezzo Output
#define redLEDpin        7     // PD7 Red LED Status
#define rxSigfox         8     // PD3 RX Serial from Sigfox modem
#define shdPin           9     // PB1 Shutdown pin
#define rstPin          10     // PB2 SS   SDCARD
//                      11     // PB3 MOSI SDCARD
//                      12     // PB4 MISO SDCARD
//                      13     // PB5 SCK  SDCARD
#define sensorA0        A0     // PC0 VUSB present
#define sensorBatt      A1     // PC1 battery voltage
#define chg500mA        A2
#define satLEDpin       A3
#define sensorA4        A4     // PC4 A4 SDA
//                      A5     // PC5 A5 SCL
#define sensorA6        A6     // PC6
#define chgFLAG         A7
/*********************************************************************/

/****************** Pins output values *******************************/
#define DIGITAL_PULLUP ((1 << shdPin) | (1 << accINT) | (1 << usbDP) | (1 << usbDM))
/*********************************************************************/

/****************** Pins direction ***********************************/
#define DDRC_MASK (1 << 2)
#define DIGITAL_OUTPUT ((1 << shdPin) | (1 << redLEDpin) | (1 << bluLEDpin) | (1 << rxSigfox) | (1 << rstPin))
/*********************************************************************/

unsigned long fix_age = 0;
int year = 0;
byte month, day, hour, minute, second, hundredths = 0;

boolean accelPresent = false;
boolean baromPresent = false;
boolean GPSactive = true;

// For automatic airplane mode detection
boolean airPlaneSpeed = false;
boolean airPlanePress = false;

uint8_t loopGPS = 0;

uint8_t limitSport = 0;
uint8_t forceSport = 0;

unsigned long start = 0;
uint8_t today = 0;
uint8_t MsgCount = 0;

byte accelPosition;
int8_t detectMotion = 1;
unsigned int batteryValue;
byte batteryPercent = 0;

uint16_t alt = 0;
uint16_t spd = 0;
uint8_t  sat = 0;
uint8_t  syncSat = 0;
uint8_t  noSat = 0;

float previous_lat = 0;
float previous_lon = 0;

// BMP180 measurements
float    Temp = 0;
uint16_t Press = 0;

// 12 octets = 96 bits payload
// lat: 32, lon: 32, alt: 13 , spd: 7, bat: 7, mode: 3, cap: 2
// lat: 32, lon: 32, alt:0-8192m, spd:0-127Km/h, bat:0-100%, mode:0-7, cap: N/E/S/W
// int is 16 bits, float is 32 bits. All little endian
struct Payload {
  float lat;
  float lon;
  uint32_t cpx;
};

Payload p;

/* Hardware and Software Changes Log

  2016-03-20
  - Remove unused variables
  - Change calibration algorithm in battery sketch. After reboot, always need a full charge cycle
  - Change detectmotion algorithm, add motion when GPS speed > 15Km/h

  2016-02-27
  - Direct port manipulation code

  2016-02-04
  - Fix loop duration during GPS process
  - New speed algorithm

  2016-01-17
  - Fix Temp and Press not updated before each non GPS message

  2016-01-14
  - Use new barometer library from seeedstudio and save 1Kb of flash program space

  2015-10-08
  - Fix standby current excessive consumption from 2mA to 0,2mA (D5)

  2015-09-10
  - Fix battery calibration EEPROM address not correct

  2015-08-26
  - Add Temperature and pressure measuremements from BMP180
  - Change Bootloader to always enter in DFU mode and exit with TIMEOUT
  - Add HidnSeek dance to shutdown the device

  2015-08-17
  - Add Eeprom datalog

  2015-08-15
  - New Hardware release 3.24
  - Fix: Avoid GPS go to sleep when green LED is high

  2015-07-31
  - Fix initial start sequence when battery is low

*/


