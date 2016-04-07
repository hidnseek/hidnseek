/*
 * MMA8653 library
 * (C) 2012 Akafugu Corporation
 *
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 */


#include "MMA8653.h"


MMA8653::MMA8653(uint8_t addr)
{
  _addr = addr;
  _rad2deg = 180.0 / M_PI;
}


//begin private methods


#define MMA_8653_CTRL_REG1 0x2A
#define MMA_8653_CTRL_REG1_VALUE_ACTIVE 0x01
#define MMA_8653_CTRL_REG1_VALUE_F_READ 0x02

#define MMA_8653_CTRL_REG2 0x2B
#define MMA_8653_CTRL_REG2_RESET 0x40

#define MMA_8653_CTRL_REG3 0x2C
#define MMA_8653_CTRL_REG3_VALUE_OD 0x01

#define MMA_8653_CTRL_REG4 0x2D
#define MMA_8653_CTRL_REG4_VALUE_INT_ASLP 0x80
#define MMA_8653_CTRL_REG4_VALUE_INT_ENLP 0x10
#define MMA_8653_CTRL_REG4_VALUE_INT_FFMT 0x04
#define MMA_8653_CTRL_REG4_VALUE_INT_DRDY 0x01

#define MMA_8653_CTRL_REG5 0x2E // 1: routed to INT1

#define MMA_8653_PL_STATUS 0x10
#define MMA_8653_PL_CFG 0x11
#define MMA_8653_PL_EN 0x40


#define MMA_8653_XYZ_DATA_CFG 0x0E
#define MMA_8653_2G_MODE 0x00 //Set Sensitivity to 2g
#define MMA_8653_4G_MODE 0x01 //Set Sensitivity to 4g
#define MMA_8653_8G_MODE 0x02 //Set Sensitivity to 8g


#define MMA_8653_FF_MT_CFG 0x15
#define MMA_8653_FF_MT_CFG_ELE 0x80
#define MMA_8653_FF_MT_CFG_OAE 0x40
#define MMA_8653_FF_MT_CFG_XYZ 0x38


#define MMA_8653_FF_MT_SRC 0x16
#define MMA_8653_FF_MT_SRC_EA 0x80

#define MMA_8653_FF_MT_THS 0x17

#define MMA_8653_FF_MT_COUNT 0x18

#define MMA_8653_PULSE_CFG 0x21
#define MMA_8653_PULSE_CFG_ELE 0x80


#define MMA_8653_PULSE_SRC 0x22
#define MMA_8653_PULSE_SRC_EA 0x80

// Sample rate
#define MMA_8653_ODR_800  0x00
#define MMA_8653_ODR_400  0x08
#define MMA_8653_ODR_200  0x10
#define MMA_8653_ODR_100  0x18 // default ratio 100 samples per second
#define MMA_8653_ODR_50   0x20
#define MMA_8653_ODR_12_5 0x28
#define MMA_8653_ODR_6_25 0x30
#define MMA_8653_ODR_1_56 0x38

uint8_t MMA8653::_read_register(uint8_t offset)
{
	Wire.beginTransmission(_addr);
	Wire.write(offset);
	Wire.endTransmission(false);


	Wire.requestFrom(_addr, (uint8_t)1);


	if (Wire.available()) return Wire.read();
	return 0;
}


void MMA8653::_write_register(uint8_t b, uint8_t offset)
{
	Wire.beginTransmission(_addr);
	Wire.write(offset);
	Wire.write(b);
	Wire.endTransmission();
}


void MMA8653::initMotion()
{
	standby();
	_write_register(MMA_8653_FF_MT_CFG, MMA_8653_FF_MT_CFG_XYZ);
	_write_register(MMA_8653_FF_MT_THS, 0x04);
	_write_register(MMA_8653_FF_MT_COUNT, 0x00);
	_write_register(MMA_8653_CTRL_REG3, MMA_8653_CTRL_REG3_VALUE_OD);
	_write_register(MMA_8653_CTRL_REG4, MMA_8653_CTRL_REG4_VALUE_INT_FFMT);
	_write_register(MMA_8653_CTRL_REG5, MMA_8653_CTRL_REG4_VALUE_INT_FFMT);
	active();
// return (_read_register(MMA_8653_PULSE_SRC) & MMA_8653_PULSE_SRC_EA);
}



void MMA8653::standby()
{
  uint8_t reg1 = 0x00;
  Wire.beginTransmission(_addr); // Set to status reg
  Wire.write((uint8_t)MMA_8653_CTRL_REG1);
  Wire.endTransmission();
  
  Wire.requestFrom((uint8_t)_addr, (uint8_t)1);
  if (Wire.available())
  {
    reg1 = Wire.read();
  }
  Wire.beginTransmission(_addr); // Reset
  Wire.write((uint8_t)MMA_8653_CTRL_REG1);
  Wire.write(reg1 & ~MMA_8653_CTRL_REG1_VALUE_ACTIVE);
  Wire.endTransmission();
}


void MMA8653::active()
{
  uint8_t reg1 = 0x00;
  Wire.beginTransmission(_addr); // Set to status reg
  Wire.write((uint8_t)MMA_8653_CTRL_REG1);
  Wire.endTransmission();
  
  Wire.requestFrom((uint8_t)_addr, (uint8_t)1);
  if (Wire.available())
  {
    reg1 = Wire.read();
  }
  Wire.beginTransmission(_addr); // Reset
  Wire.write(MMA_8653_CTRL_REG2);
  Wire.write(0x09);
  Wire.endTransmission();

  Wire.beginTransmission(_addr); // Reset
  Wire.write((uint8_t)MMA_8653_CTRL_REG1);
  Wire.write(reg1 | MMA_8653_CTRL_REG1_VALUE_ACTIVE | (_highres ? 0 : MMA_8653_CTRL_REG1_VALUE_F_READ) | MMA_8653_ODR_6_25);
  Wire.endTransmission();
}


float MMA8653::geta2d(float gx, float gy)
{
  float a;
  
  a = gx * gx;
  a = fma(gy,gy,a);
  
  return sqrt(a);
}


//gets the magnitude of the 3d vector
//the formula is a^2 = x^2 + y^2 + z^2
float MMA8653::geta3d(float gx, float gy, float gz)
{
  float a;
  
  //use floating point multiply-add cpu func
  //sometimes we get better precision
  a = gx * gx;
  a = fma(gy,gy,a);
  a = fma(gz,gz,a);
  
  return sqrt(a);
}


float MMA8653::_getRho(float ax, float ay, float az)
{
  return geta3d(_xg,_yg,_zg);
}


float MMA8653::_getPhi(float ax, float ay, float az)
{
  return atan2(ay, ax) * _rad2deg;  
}


float MMA8653::_getTheta(float ax, float ay, float az)
{
  float rho = _getRho(ax, ay, az);
  
  if (rho == 0.0)
    return NAN;
  else
    return acos(az / rho) * _rad2deg;
}


//end private methods


//begin public methods
void MMA8653::begin(bool highres, uint8_t scale)
{
  _highres = highres;
  
  _scale = scale;
#ifdef _MMA_8653_FACTOR
  _step_factor = (_highres ? 0.0039 : 0.0156); // Base value at 2g setting
  if( _scale == 4 )
    _step_factor *= 2;
  else if (_scale == 8)
    _step_factor *= 4;
#endif

  uint8_t wai = _read_register(0x0D); // Get Who Am I from the device.
  // return value for MMA8543Q is 0x3A
  
  Wire.beginTransmission(_addr); // Reset
  Wire.write(MMA_8653_CTRL_REG2);
  Wire.write(MMA_8653_CTRL_REG2_RESET);
  Wire.endTransmission();
  delay(10); // Give it time to do the reset
  standby();

#ifdef _MMA_8653_PORTRAIT_LANDSCAPE
  Wire.beginTransmission(_addr); // Set Portrait/Landscape mode
  Wire.write(MMA_8653_PL_CFG);
  Wire.write(0x80 | MMA_8653_PL_EN);
  Wire.endTransmission();
#endif

  Wire.beginTransmission(_addr);
  Wire.write(MMA_8653_XYZ_DATA_CFG);
  if (_scale == 4 || _scale == 8)
    Wire.write((_scale == 4) ? MMA_8653_4G_MODE : MMA_8653_8G_MODE);
  else // Default to 2g mode
    Wire.write((uint8_t)MMA_8653_2G_MODE);
  Wire.endTransmission();
  active();
}


uint8_t MMA8653::getPLStatus()
{
	return _read_register(MMA_8653_PL_STATUS);
}


uint8_t MMA8653::getPulse()
{
	_write_register(MMA_8653_PULSE_CFG, MMA_8653_PULSE_CFG_ELE);
	return (_read_register(MMA_8653_PULSE_SRC) & MMA_8653_PULSE_SRC_EA);
}


float MMA8653::getXG()
{
  return _xg;
}


float MMA8653::getYG()
{
  return _yg;
}


float MMA8653::getZG()
{
  return _zg;
}

int8_t MMA8653::getX()
{
  return _x;
}

int8_t MMA8653::getY()
{
  return _y;
}

int8_t MMA8653::getZ()
{
  return _z;
}

float MMA8653::getRho()
{
  return _getRho(_xg,_yg,_zg);
}


float MMA8653::getPhi()
{
  return _getPhi(_xg,_yg,_zg);
}


float MMA8653::getTheta()
{
  return _getTheta(_xg,_yg,_zg);
}


int16_t rx, ry, rz;


byte MMA8653::update()
{
  Wire.beginTransmission(_addr); // Set to status reg
  Wire.write((uint8_t)0x00);
  byte error = Wire.endTransmission(false);
  
  Wire.requestFrom((uint8_t)_addr, (uint8_t)(_highres ? 7 : 4));
  if (Wire.available()) 
  {
    _stat = Wire.read();
    if(_highres)
    {
      _x = (int16_t)((Wire.read() << 8) + Wire.read());
      _y = (int16_t)((Wire.read() << 8) + Wire.read());
      _z = (int16_t)((Wire.read() << 8) + Wire.read());
#ifdef _MMA_8653_FACTOR
      _xg = (_x / 64) * _step_factor;
      _yg = (_y / 64) * _step_factor;
      _zg = (_z / 64) * _step_factor;
#endif
    }
    else
    {
      _x = Wire.read();
      _y = Wire.read();
      _z = Wire.read();
#ifdef _MMA_8653_FACTOR
      _xg = _x*_step_factor;
      _yg = _y*_step_factor;
      _zg = _z*_step_factor;
#endif
    }
  }
  return error;
}

bool MMA8653::setInterrupt(uint8_t type, uint8_t pin, bool on)
{
	uint8_t current_value = _read_register(0x2D);

	if(on)
		current_value |= type;
	else
		current_value &= ~(type);

	_write_register(0x2D, current_value);

	uint8_t current_routing_value = _read_register(0x2E);

	if (pin == 1) {
		current_routing_value &= ~(type);
	}
	else if (pin == 2) {
		current_routing_value |= type;
	}

	_write_register(0x2E, current_routing_value);
}

bool MMA8653::disableAllInterrupts()
{
	_write_register(0x2D, 0);
}

//end public methods

