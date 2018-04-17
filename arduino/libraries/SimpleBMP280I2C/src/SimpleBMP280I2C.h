/*
 * BMP280 barometric pressure sensor library for the Arduino
 * microcontroller.
 *
 * uses the I2C library from Wayne Truchsess
 *
 * Copyright (C) 2016 Edwin Croissant
 *
 *  This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See the README.md file for additional information.
 */

#ifndef SIMPLEBMP280I2C_H
#define SIMPLEBMP280I2C_H

#include "Arduino.h"
#include "I2C.h" // https://github.com/rambo/I2C

class SimpleBMP280I2C {
public:
	SimpleBMP280I2C(int8_t = 0x76);
	boolean begin();
	boolean begin(int8_t);
	int32_t getPressure(void);
	int16_t getLastTemperature(void);

private:
	int8_t _I2C_Addr;
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

#endif //  SIMPLEBMP280I2C_H
