/*
 * Barometer.h
 * A library for barometer bmp180
 *
 * Copyright (c) 2012 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : LG
 * Create Time:
 * Change Log :
 *
 * loovee 9-24-2014
 * Change all int to short, all unsigned int to unsigned short to fit some 32bit system
 *
 * The MIT License (MIT)
 *
 * BMP280 barometric pressure sensor library for the Arduino
 * microcontroller.
 *
 * uses the I2C library from Wayne Truchsess
 *
 * Copyright (C) 2016 Edwin Croissant
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef __BAROMETER_H__
#define __BAROMETER_H__

#include <Arduino.h>
#include <I2C.h>

const unsigned char OSS = 0;
#define BMP085_ADDRESS 0x77
#define BMP280_ADDRESS 0x77

class Barometer
{
    public:
    bool init(void);
    long PressureCompensate;
    float bmp085GetTemperature();
    long bmp085GetPressure();
    float calcAltitude(float pressure);
    unsigned short bmp085ReadUT(void);
    unsigned long bmp085ReadUP(void);

	bool begin();
	uint32_t getPressure(void);
	int16_t getLastTemperature(void);

    private:

    short ac1;
    short ac2;
    short ac3;
    unsigned short ac4;
    unsigned short ac5;
    unsigned short ac6;
    short b1;
    short b2;
    short mb;
    short mc;
    short md;
    uint8_t bmp085Read(uint8_t address);
    short bmp085ReadInt(uint8_t address);

    //uint8_t _I2C_Addr;
	int16_t _lastTemp;

	union {
		uint8_t calArray[24];
		struct {
			uint16_t dig_T1;
			int16_t dig_T2, dig_T3;
			uint16_t dig_P1;
			int16_t dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
		};
	} _calData;

	enum BMP280registers {
		calData = 0x88, id = 0xD0, control = 0xF4, config = 0xF5, data = 0xF7
	};

	enum BMP280constants {
		deviceId = 0x58
	};
};

#endif
