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

#define ALTITUDE 252.0 // Altitude of HidnSeek's HQ in Grenoble in meters

bool bmp280Measure(float *Temp, float *Press)
{
  static unsigned int GroundPress;
  *Press = bmp280.getPressure() / 100.0;
  if ((*Press > GroundPress) || (batteryPercent > 90) || (syncSat > 0 && airPlaneSpeed == false)) GroundPress = *Press;
  airPlanePress = (GroundPress  >= *Press + 32) ? true : false;
    *Temp = bmp280.getLastTemperature() / 100.0; // Return -273°C if sensor don't answer
  return true;
}

void bmp280Print()
{
  serialString(PSTR("Temp: "));
  Serial.print(Temp, 2);
  serialString(PSTR("'C Press: "));
  Serial.print(Press,1);
  serialString(PSTR("mb\r\n"));
}
