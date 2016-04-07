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
#ifndef MMA8653_H
#define MMA8653_H


#define MMA_8653_DEFAULT_ADDRESS 0x1D


#include "Arduino.h"
#include "Wire.h"


////////////////////////////////////////////
// uncomment to add portrait/Landscape interrupt
// #define _MMA_8653_PORTRAIT_LANDSCAPE

// uncomment to enable G factor outputs
// #define _MMA_8653_FACTOR

////////////////////////////////////////////
// Interrupts


// Auto SLEEP/WAKE interrupt
#define INT_ASLP   (1<<7)
// Transient interrupt
#define INT_TRANS  (1<<5)
// Orientation (landscape/portrait) interrupt
#define INT_LNDPRT (1<<4)
// Pulse detection interrupt
#define INT_PULSE  (1<<3)
// Freefall/Motion interrupt
#define INT_FF_MT  (1<<2)
// Data ready interrupt
#define INT_DRDY   (1<<0)




class MMA8653
{
  public:
    MMA8653(uint8_t addr = MMA_8653_DEFAULT_ADDRESS);
    void begin(bool highres = true, uint8_t scale = 2);
    float getXG();
    float getYG();
    float getZG();
    int8_t getX();
    int8_t getY();
    int8_t getZ();
    float getRho();
    float getPhi();
    float getTheta();
    byte update();
    
    uint8_t getPLStatus();
    uint8_t getPulse();
    
    // Interrupts
    bool setInterrupt(uint8_t type, uint8_t pin, bool on);
    bool disableAllInterrupts();
    void initMotion();
    void standby();
    void active();

  private:
    uint8_t _read_register(uint8_t offset);
    void _write_register(uint8_t b, uint8_t offset);
    
    float geta2d(float gx, float gy);
    float geta3d(float gx, float gy, float gz);
    float _getRho(float ax, float ay, float az);
    float _getPhi(float ax, float ay, float az);
    float _getTheta(float ax, float ay, float az);
    
    uint8_t _addr;
    uint8_t _stat;
    uint8_t _scale;
    int8_t _x;
    int8_t _y;
    int8_t _z;
    float _step_factor;
    bool _highres;
    float _xg;
    float _yg;
    float _zg;
    float _rad2deg;
};

#endif
