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

bool bmp180Measure(float *Temp, unsigned int *Press)
{
  static unsigned int GroundPress;
  if (Temp) *Temp = bmp180.bmp085GetTemperature(bmp180.bmp085ReadUT()) - 3.6;
  if (Press) {
    *Press = (unsigned int) (bmp180.bmp085GetPressure(bmp180.bmp085ReadUP()) / 100);
    if ((*Press > GroundPress) || (syncSat > 0 && airPlaneSpeed == false)) GroundPress= *Press;
    airPlanePress = (unsigned int)(GroundPress - *Press) >= 32 ? true : false;
  }
  return true;
}

void bmp180Print()
{
  serialString(PSTR("Temp: "));
  Serial.print(Temp, 2);
  serialString(PSTR("'C Press: "));
  Serial.print(Press);
  serialString(PSTR("mb\r\n"));
}
