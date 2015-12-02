// **** INCLUDES *****
#include "LowPower.h"

/****************** Pins usage ***************************************/
#define recPin           0     // PD0 RX Serial from GPS
//                       1     // PD1 TX Serial to GPS
#define usbDP            2     // PD2 Shutdown supply after power off
#define accINT           3     // PD3 Accelerometer Interruption
#define usbDM            4     // PD4
#define txSigfox         5     // PD5 TX Serial to Sigfox modem
#define piezzo           6     // PD6 Piezzo Output
#define bluLEDpin        6     // PD6 Piezzo Output
#define redLEDpin        7     // PD7 Red LED Status
#define rxSigfox         8     // PB0 RX Serial from Sigfox modem
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

#include "Wire.h"
#include "MMA8653.h"

MMA8653 accel;

void initGPIO()
{
  // Set output to 0 for not used Pads
  pinMode(sensorA0, OUTPUT);
  digitalWrite(sensorA0, HIGH);

  //  pinMode(piezzo, OUTPUT);
  //  digitalWrite(piezzo, LOW);

  pinMode(chg500mA, OUTPUT);
  digitalWrite(chg500mA, LOW);

  pinMode(redLEDpin, OUTPUT);
  digitalWrite(redLEDpin, LOW);

  pinMode(bluLEDpin, INPUT);
  digitalWrite(redLEDpin, HIGH);

  digitalWrite(rstPin, LOW);
  pinMode(rstPin, OUTPUT);

  pinMode(satLEDpin, INPUT);

  pinMode(accINT, INPUT);
  digitalWrite(accINT, HIGH);

  pinMode(usbDM, INPUT);
  digitalWrite(usbDM, HIGH);

  pinMode(usbDP, INPUT);
  digitalWrite(usbDP, LOW);

  digitalWrite(shdPin, HIGH);
  pinMode(shdPin, OUTPUT);

  for (uint8_t i = 11; i < 14; i++) {
    digitalWrite (i, HIGH);
    pinMode (i, INPUT);
  }
}

void setup()
{
  initGPIO();
  //Wire.begin();
  //accel.begin(false, 2);
}

void flash()
{
  int led;
  static int previous;
  //byte error = accel.update();
  int angle = accel.getZ();
  if (previous != angle) led = bluLEDpin; else led = redLEDpin;
  digitalWrite (led, HIGH);
  delay(25);
  digitalWrite (led, LOW);
  previous = angle;
}

void flashR()
{
  digitalWrite (bluLEDpin, LOW);
  digitalWrite (redLEDpin, HIGH);
  delay(25);
  digitalWrite (redLEDpin, LOW);
}

void flashB()
{
  digitalWrite (bluLEDpin, HIGH);
  delay(25);
}

void loop()
{
  // Enter power down state for 8 s with ADC and BOD module disabled
  flashB();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  flashR();
  delay(8000);
  // Do something here
  // Example: Read sensor, data logging, data transmission.
}
