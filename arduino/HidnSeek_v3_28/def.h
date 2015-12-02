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

#define ACCEL_MODE 2
#define ACCEL_TRIG 40
#define ACCEL_FLAT 10

#define MsgMax     140   // 12 messages can be sent per hour.
#define MOTION_MIN_NUMBER 2
#define PMTK_SET_NMEA_OUTPUT_RMCONLY "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*" // 29"
#define PMTK_SET_NMEA_OUTPUT_RMCGGA  "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*" // 28"
#define PMTK_SET_NMEA_OUTPUT_RMCGGA5 "$PMTK314,0,1,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*" // 2C"
#define PMTK_SET_NMEA_OUTPUT_DATA    "$PMTK314,0,3,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*" // 29"
#define PMTK_SET_NMEA_OUTPUT_ALLDATA "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*" // 28"

#define PMTK_AWAKE   "$PMTK010,001*" // 2E"
#define PMTK_STANDBY "$PMTK161,0*"   // 28"
#define PMTK_VERSION "$PMTK605*"     // 31"
#define PMTK_SET_NMEA_UPDATE_1HZ "$PMTK220,1000*" // 1F"
#define PMTK_ENABLE_SBAS "$PMTK313,1*" // 2E"
#define PMTK_ENABLE_WAAS "$PMTK301,2*" // 2E"

enum {
  MSG_POSITION = 0, MSG_CE_CW = 3, MSG_NO_MOTION = 4, MSG_NO_GPS = 5,
  MSG_MOTION_ALERT = 6, MSG_WEAK_BAT = 7
};

enum {
  POS_FACE_UP = 0, POS_FACE_DN = 1, POS_SIDE_UP = 2,
  POS_SIDE_DN = 3, POS_SIDE_RT = 4, POS_SIDE_LT = 5,
  POS_NULL = 6
};

/****************** Pins usage ***************************************/
#define recPin           0     // PD0 RX Serial from GPS
//                       1     // PD1 TX Serial to GPS
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

unsigned char buffer[32];

unsigned long fix_age, date, time, chars = 0;
int year;
byte month, day, hour, minute, second, hundredths;
byte msgs = 0;
boolean modemPresent = false;
boolean accelPresent = false;
boolean baromPresent = false;
boolean GPSactive = true;

unsigned long loopCW = millis();
unsigned long start = millis();
unsigned long timer = millis();
unsigned long fixTime = millis();
uint8_t today = 0;
uint8_t MsgCount = 0;

int detectMotion = 1;
int waitLoop = 80;
int batteryValue;
int batteryCharge;
int batteryLow = 3570;
int syncSat = 0;
int noSat = 0;
byte batteryPercent = 0;
int wakeMsgs = 0;

float previous_lat = 0;
float previous_lon = 0;

// BMP180 measurements
float Temp, Press, p0;

// 12 octets = 96 bits payload
// lat: 32, lon: 32, alt: 13 , spd: 7, bat: 7, mode: 3, cap: 2
// lat: 32, lon: 32, alt:0-8192m, spd:0-127Km/h, bat:0-100%, mode:0-7, cap: N/E/S/W
// int is 16 bits, float is 32 bits. All little endian
typedef struct {
  float lat;
  float lon;
  uint32_t cpx;
}
Payload;

Payload p;

/* Hardware and Software Changes Log

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


