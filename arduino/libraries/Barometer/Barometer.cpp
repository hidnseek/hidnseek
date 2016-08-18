/*
 * Barometer.cpp
 * A library for barometer
 *
 * Copyright (c) 2012 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : LG
 * Create Time:
 * Change Log :
 * 
 * loovee 9-24-2014
 * Change all int to short, all unsigned int to unsigned short to fit some 32bit system
 * The MIT License (MIT)
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

#include "Barometer.h"

bool Barometer::init(void)
{
    I2c.begin();
    ac1 = bmp085ReadInt(0xAA);
    ac2 = bmp085ReadInt(0xAC);
    ac3 = bmp085ReadInt(0xAE);
    ac4 = bmp085ReadInt(0xB0);
    ac5 = bmp085ReadInt(0xB2);
    ac6 = bmp085ReadInt(0xB4);
    b1 = bmp085ReadInt(0xB6);
    b2 = bmp085ReadInt(0xB8);
    mb = bmp085ReadInt(0xBA);
    mc = bmp085ReadInt(0xBC);
    md = bmp085ReadInt(0xBE);
    return (bmp085Read(0xD0)==0x55);
}

// Read 1 byte from the BMP085 at 'address'
// Return: the read byte;
uint8_t Barometer::bmp085Read(uint8_t address)
{
    I2c.read(BMP085_ADDRESS, address, 1);
    return I2c.receive();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
short Barometer::bmp085ReadInt(uint8_t address)
{
    unsigned char msb, lsb;
    if (I2c.read(BMP085_ADDRESS, address, 2) == 0) {
        msb = I2c.receive();
        lsb = I2c.receive();
    }
    return (short) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned short Barometer::bmp085ReadUT()
{
    unsigned short ut;
    I2c.write(BMP085_ADDRESS, 0xF4, 0x2E);
    delay(5);
    ut = bmp085ReadInt(0xF6);
    return ut;
}

// Read the uncompensated pressure value
unsigned long Barometer::bmp085ReadUP()
{
    uint8_t msb, lsb, xlsb;
    unsigned long up = 0;
    I2c.write(BMP085_ADDRESS, 0xF4, 0x34 + (OSS<<6));
    delay(2 + (3<<OSS));

    // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
    if (I2c.read(BMP085_ADDRESS, 0xF6, 3) == 0) {
        msb = I2c.receive();
        lsb = I2c.receive();
        xlsb = I2c.receive();
    }
    
    up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);
    return up;
}

float Barometer::calcAltitude(float pressure)
{
    float A = pressure/101325;
    float B = 1/5.25588;
    float C = pow(A,B);
    C = 1 - C;
    C = C /0.0000225577;
    return C;
}

float Barometer::bmp085GetTemperature()
{
    long x1, x2;
    unsigned short ut = bmp085ReadUT();
    x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
    x2 = ((long)mc << 11)/(x1 + md);
    PressureCompensate = x1 + x2;

    float temp = ((PressureCompensate + 8)>>4);
    temp = temp /10;

    return temp;
}

long Barometer::bmp085GetPressure()
{
    unsigned long up = bmp085ReadUP();
    long x1, x2, x3, b3, b6, p;
    unsigned long b4, b7;
    b6 = PressureCompensate - 4000;
    x1 = (b2 * (b6 * b6)>>12)>>11;
    x2 = (ac2 * b6)>>11;
    x3 = x1 + x2;
    b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

    // Calculate B4
    x1 = (ac3 * b6)>>13;
    x2 = (b1 * ((b6 * b6)>>12))>>16;
    x3 = ((x1 + x2) + 2)>>2;
    b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

    b7 = ((unsigned long)(up - b3) * (50000>>OSS));
    if (b7 < 0x80000000UL)
        p = (b7<<1)/b4;
    else
        p = (b7/b4)<<1;

    x1 = (p>>8) * (p>>8);
    x1 = (x1 * 3038)>>16;
    x2 = (-7357 * p)>>16;
    p += (x1 + x2 + 3791)>>4;

    return p;
}

boolean Barometer::begin() {

	/* check device */
	if (I2c.read(BMP280_ADDRESS, id, 1) != 0)
		return false;
	if (I2c.receive() != deviceId)
		return false;

	/* read the calibration data*/
	if (I2c.read(BMP280_ADDRESS, calData, 24) != 0)
		return false;
	for (int i = 0; i < 24; ++i) {
		_calData.calArray[i] = I2c.receive();
	}

	/* set the mode */
	if (I2c.write(BMP280_ADDRESS, config, 0b10100000) != 0)
		return false;
	if (I2c.write(BMP280_ADDRESS, control, 0b00100111) != 0)
		return false;

	return true;
}

/*
 * Returns the pressure in Pascal.
*/
uint32_t Barometer::getPressure(void) {
	int32_t var1, var2, t_fine;
	uint32_t p;

	union {
		int32_t int32;
		struct {
			uint8_t XLSB, LSB, MSB;
		};
	} UT, UP; //uncompensated temperature and uncompensated pressure

	if (I2c.read(BMP280_ADDRESS, data, 6) != 0) {
		_lastTemp = -27315;
		return 0;
	}

	UP.MSB = I2c.receive();
	UP.LSB = I2c.receive();
	UP.XLSB = I2c.receive();
	UT.MSB = I2c.receive();
	UT.LSB = I2c.receive();
	UT.XLSB = I2c.receive();

	UT.int32 >>= 4;
	UP.int32 >>= 4;

	var1 = ((((UT.int32 >> 3) - ((int32_t) _calData.dig_T1 << 1)))
			* ((int32_t) _calData.dig_T2)) >> 11;

	var2 = (((((UT.int32 >> 4) - ((int32_t) _calData.dig_T1))
			* ((UT.int32 >> 4) - ((int32_t) _calData.dig_T1))) >> 12)
			* ((int32_t) _calData.dig_T3)) >> 14;

	t_fine = var1 + var2;
	_lastTemp = (t_fine * 5 + 128) >> 8;

	var1 = (t_fine >> 1) - (int32_t) 64000;

	var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t) _calData.dig_P6);

	var2 = var2 + ((var1 * ((int32_t) _calData.dig_P5)) << 1);
	var2 = (var2 >> 2) + (((int32_t) _calData.dig_P4) << 16);
	var1 = (((_calData.dig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3)
			+ ((((int32_t) _calData.dig_P2) * var1) >> 1)) >> 18;
	var1 = ((((32768 + var1)) * ((int32_t) _calData.dig_P1)) >> 15);

	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}

	p = (((uint32_t) (((int32_t) 1048576) - UP.int32) - (var2 >> 12))) * 3125;

	if (p < 0x80000000) {
		p = (p << 1) / ((uint32_t) var1);
	} else {
		p = (p / (uint32_t) var1) * 2;
	}

	var1 = (((int32_t) _calData.dig_P9)
			* ((int32_t) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
	var2 = (((int32_t) (p >> 2)) * ((int32_t) _calData.dig_P8)) >> 13;
	p = (uint32_t) ((int32_t) p + ((var1 + var2 + _calData.dig_P7) >> 4));

	return p; // in Pascal
}

/*
 * Retrieves the sensor temperature in centiCelsius as
 * measured during the pressure reading.
*/
int16_t Barometer::getLastTemperature(void) {
	return _lastTemp;
}