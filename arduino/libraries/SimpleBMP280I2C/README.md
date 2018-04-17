#SimpleBMP280I2C library

This is a bare bone BMP280 Library

This library uses 32 bit fixed point calculations.
The sensor is set at the ultra low power setting and a 1 second refresh rate.

##Dependency

This library uses the [I2C library from Wayne Truchsess] (https://github.com/rambo/I2C)
The example compiles to about 520 bytes less code and uses 169 bytes less dynamic memory compared to the Wire library and won't freeze when the sensor is disconnected.

##Usage

* begin()

 Check the device and retrieves the calibration data.  Returns false if the operation fails.

* getPressure(mode)

 Initiate a temperature and pressure reading and returns the pressure in Pascal.  If there is a I2C error returns 0.

* getLastTemperature()

 Retrieves the sensor temperature in deci Celsius as measured during the pressure reading.  If there was
a I2C error returns -2732.

See the example how to use this library.

 
