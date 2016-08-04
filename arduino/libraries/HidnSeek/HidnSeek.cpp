/* This file started from Akeru library http://akeru.cc copyleft Snootlab, 2014
 and has been modified for HidnSeek by Stephane D, 2014.

 This library is free software: you can redistribute it and/or
 modify it under the terms of the GNU General Public License as published
 by the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with HidnSeek.  If not, see <http://www.gnu.org/licenses/>.*/

#include <Arduino.h>
#include "HidnSeek.h"

HidnSeek::HidnSeek(uint8_t rxPin, uint8_t txPin) :
    _serial(rxPin, txPin) {
     //Since _lastSend is unsigned, this is infinity
    _lastSend = -1;
}

HidnSeek::~HidnSeek() {
}

int HidnSeek::begin() {
    _serial.begin(9600);

    //Remove un-ended commands from TST's buffer
    _serial.write((uint8_t)'\0');
    _serial.write((uint8_t)';');

    char dataRX[5] = "";
    int  length    = 3;
    int  timeout   = 50;
    int  bread     = 0;

    long previousMillis = millis();
    while((millis()-previousMillis)<timeout)
    {
        if(_serial.available()>0)
        {
            char c = _serial.read();
            if(bread>length)
            {
                return(-1); // string received is too long
            }
            else
            {
                dataRX[bread]=c;
            }
            bread++;
        }
    }
    if (strcmp (dataRX,"KO;") == 0) return(bread); else return(-1);
}

bool HidnSeek::isReady() {

	// IMPORTANT WARNING. PLEASE READ BEFORE MODIFYING THE CODE
	//
	// The Sigfox network operates on public frequencies. To comply with
	// radio regulation, it can send radio data a maximum of 1% of the time
	// to leave room to other devices using the same frequencies.
	//
	// Sending a message takes about 6 seconds (it's sent 3 times for
	// redundancy purposes), meaning the interval between messages should
	// be 10 minutes.
	//
	// Also make sure your send rate complies with the restrictions set
	// by the particular subscription contract you have with your Sigfox
	// network operator.
	//
	// FAILING TO COMPLY WITH THESE CONSTRAINTS MAY CAUSE YOUR MODEM
	// TO BE BLOCKED BY YOUR SIFGOX NETWORK OPERATOR.
	//
	// You've been warned!

    unsigned long currentTime = millis();
    if(currentTime >= _lastSend && ((unsigned long)(currentTime - _lastSend) <= 600000UL)) {
        return false;
    }

    // Time is ok, ask the modem's status
    _serial.write((uint8_t)'\0');
    _serial.write((uint8_t)'S');
    _serial.write((uint8_t)'F');
    _serial.write((uint8_t)'P');
    _serial.write((uint8_t)';');

    return _nextReturn() == OK;
}

bool HidnSeek::send(const void* data, uint8_t len) {
	uint8_t* bytes = (uint8_t*)data;

/*    if(!isReady()) {
        return false;
    } */

    // See comment in isReady()
    _lastSend = millis();

    _serial.write((uint8_t)'\0');
    _command(PSTR("SFM"));
    _serial.write(len);
    for(uint8_t i = 0; i < len; ++i) {
        _serial.write(bytes[i]);
    }
    _serial.write(';');

    uint8_t ok = _nextReturn();
    if(ok == OK) {
        _nextReturn(); //SENT
        return true;
    }
    return false;
}

uint8_t HidnSeek::getRev() {
    _serial.write((uint8_t)'\0');
    _serial.write((uint8_t)'S');
    _serial.write((uint8_t)'F');
    _serial.write((uint8_t)'v');
    _serial.write((uint8_t)';');

    while(_serial.available()<3);

    if(_serial.peek() == 'K') {
        _serial.read(); //'K'
        _serial.read(); //'O'
        _serial.read(); //';'
        return 0;
    } else {
        while(_serial.available()<5);
        uint8_t rev = 10 * (_serial.read() - '0') + (_serial.read() - '0');

        _serial.read(); //'O'
        _serial.read(); //'K'
        _serial.read(); //';'

        return rev;
    }
}

void HidnSeek::_command (PGM_P s) {
  char c;
  while ((c = pgm_read_byte(s++)) != 0) {
    _serial.print(c);
  }
}

unsigned long HidnSeek::getID() {
    _serial.write((uint8_t)'\0');
    _command(PSTR("SFID;"));

    //Response is [byte1, byte2, ..., byteN, 'O', 'K']
    uint8_t response[8] = {0};
    uint8_t i = 0;
    while(!_serial.available());
    while(_serial.peek() != ';' || i<6) {
        response[i] = _serial.read();
        while(!_serial.available());
        ++i;
    }
    _serial.read(); //';'

    unsigned long id = 0;

    for(uint8_t j = 0; j < i-2; ++j) {
        id = (id << 8) + response[j];
    }

    return id;
}

//Power value:
bool HidnSeek::setPower(uint8_t power) {
    // 13,9dBm with 0,4,47 for the parameter
    if (power > 0) _command(PSTR("AT$MT=0,4,47")); else {
      _command(PSTR("ATZ"));
    }
    _serial.println();

    char dataRX[5] = "";
    int  length    = 3;
    int  timeout   = 50;
    int  bread     = 0;

    long previousMillis = millis();
    while((millis()-previousMillis)<timeout)
    {
        if(_serial.available()>0)
        {
            char c = _serial.read();
            if(bread>length)
            {
                return(-1); // string received is too long
            }
            else
            {
                dataRX[bread]=c;
            }
            if (c == 'O') bread++;
            if (c == 'K') break;
        }
    }
    if (strcmp (dataRX,"OK") != 0) return false; else return true;
}

uint8_t HidnSeek::_nextReturn() {
    while(!_serial.available()) if (millis()-_lastSend > 2000) PORTD &= ~(1 << 7);
    char fstChar = _serial.read();
    while(_serial.read() != ';');
    return fstChar;
}

void HidnSeek::setSupply(boolean shd) {
    if (shd) {
        PORTB |= (1 << 1); //Pin 1 of portb is now confirm supply
        DDRB |= B00000010; //Pin 1 of portb is an output
    }
    else {
        PORTC = 0;
        DDRC = 0;
        PORTD = 0;
        DDRD = 0;
        PORTB = 0;
        DDRB = B00000010;
    }
}

void HidnSeek::initGPIO(boolean discret) {
    PORTB = B00111010;
    DDRB  = B00000111;
    DDRC  = B01000101;
    PORTC = B00000000;
    if (discret) {
        DDRD  = B00000010;
        PORTD = B00011000;
    }
    else {
        DDRD  = B11000010;
        PORTD = B00011000;
    }
}

void HidnSeek::checkBattery() {
    if (millis() - _lastCheck > 2000UL) {
        _lastCheck = millis();
        boolean _grst = PORTB & (1<<2);
        PORTB |= (1 << 2);
        if (!_grst) delay(50);
        unsigned int value = analogRead(A1);
        if (value < 900) setSupply(false);   // Below 3,7V shutdown the supply
        if (!_grst) PORTB &= ~(1<<2);
    }
}
