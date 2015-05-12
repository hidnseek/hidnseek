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

/*
uint8_t rdata;
uint8_t i;
int x, x1, y, y1, z, z1;
uint8_t stat;
String stringOutput;
*/

void accelPrint() {
  accel.update();
  Serial.print("Accel Z: ");
  Serial.println(accel.getZ());
  /* 
  Serial.println("X,\tY,\tZ,\tRho,\tPhi,\tTheta");
  Serial.print(accel.getX());
  Serial.print(" , ");
  Serial.print(accel.getY());
  Serial.print(", ");
  Serial.print(", ");
  Serial.print(accel.getRho());
  Serial.print(", ");
  Serial.print(accel.getPhi());
  Serial.print(", ");
  Serial.print(accel.getTheta());
  Serial.println();
  delay(500); 
  */
}

void initMems() {
  DEBUG_PRINTLN("Init Mems");
  Wire.begin();
  accel.begin(false, 2);
}

