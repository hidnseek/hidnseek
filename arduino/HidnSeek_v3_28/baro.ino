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

bool bmp180Measure(float *Temp, float *Press)
{
  char status;
  double T, a;

  //unsigned int alt = round(gps.f_altitude());

  status = pressure.startTemperature();
  if (status == 0) return false;
  // Wait for the measurement to complete:
  delay(status);

  // Retrieve the completed temperature measurement:
  // Note that the measurement is stored in the variable T.
  // Function returns 1 if successful, 0 if failure.

  status = pressure.getTemperature(T);
  if (status == 0) return false;
  if (Temp) *Temp = T - 3.6;
  status = pressure.startPressure(3);

  if (status == 0) return false;
  // Wait for the measurement to complete:
  delay(status);

  // Retrieve the completed pressure measurement:
  status = pressure.getPressure(a, T);
  if (status != 0) if (Press) {
      *Press = a;
      airPlanePress = a < 880 ? true : false;
    }
  return true;
}

void bmp180Print()
{
  serialString(PSTR("temp: "));
  Serial.print(Temp, 2);
  Serial.println("'C");
  serialString(PSTR("abs press: "));
  Serial.print(Press, 2);
  Serial.println("mb");
}
