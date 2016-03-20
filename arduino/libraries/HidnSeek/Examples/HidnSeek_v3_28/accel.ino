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

bool initMems() {
  serialString(PSTR("Init Mems:"));
  Wire.begin();
  accel.begin(false, ACCEL_MODE);
  if (accel.update() != 0) {
    serialString(PSTR("Fail"));
    Serial.println();
    return false;
  }
  else {
    serialString(PSTR("OK"));
    Serial.println();
    return true;
  }
}

bool accelStatus() {
  static int8_t x, y, z;
  static byte seq;
  if (accelPresent == false) return true;

  boolean accelMove = false;
  byte error = accel.update();

  if (error != 0) accelMove = true;
  else {
    accelMove = ((uint8_t)(abs((int8_t)(accel.getX() - x))) > 2) ? true :
                ((uint8_t)(abs((int8_t)(accel.getY() - y))) > 2) ? true :
                ((uint8_t)(abs((int8_t)(accel.getZ() - z))) > 2) ? true : false;
    x = accel.getX();
    y = accel.getY();
    z = accel.getZ();

    if (accelMove) { // Compute accelDance on move
      byte newPosition;
      if      (abs(x) < ACCEL_FLAT && abs(y) < ACCEL_FLAT && z > ACCEL_TRIG) newPosition = POS_FACE_UP;
      else if (abs(x) < ACCEL_FLAT && abs(y) < ACCEL_FLAT && z < -ACCEL_TRIG) newPosition = POS_FACE_DN;
      else if (abs(x) < ACCEL_FLAT && y < -ACCEL_TRIG && abs(z) < ACCEL_FLAT) newPosition = POS_SIDE_UP;
      else if (abs(x) < ACCEL_FLAT && y > ACCEL_TRIG && abs(z) < ACCEL_FLAT) newPosition = POS_SIDE_DN;
      else if (x > ACCEL_TRIG && abs(y) < ACCEL_FLAT && abs(z) < ACCEL_FLAT) newPosition = POS_SIDE_RT;
      else if (x < -ACCEL_TRIG && abs(y) < ACCEL_FLAT && abs(z) < ACCEL_FLAT) newPosition = POS_SIDE_LT;
      else {
        seq = 0;
        newPosition = POS_NULL;
      }
      if (seq == 0 && accelPosition == POS_FACE_UP && newPosition == POS_SIDE_UP) {
        seq++;
        NoflashRed();
      }
      else if (seq == 1 && accelPosition == POS_SIDE_UP && newPosition == POS_SIDE_DN) {
        seq++;
        NoflashRed();
      }
      else if (seq == 2 && accelPosition == POS_SIDE_DN && newPosition == POS_SIDE_UP) {
        seq++;
        NoflashRed();
      }
      else if (seq == 2 && accelPosition == POS_SIDE_DN && newPosition == POS_FACE_UP) {
        seq += 2;
        NoflashRed();
      }
      else seq = 0;
      accelPosition = newPosition;
      if (seq == 3) { // seq == 3
        saveEEprom();
        flashRed(20);
        PORTC = 0;
        DDRC = 0;
        PORTD = 0;
        DDRD = 0;
        PORTB = 0;
        DDRB = B00000010;
        seq = 0;
      }
      if (seq == 4) {
        if (MsgCount < 90) {
          forceSport = 1 - forceSport;
          limitSport = 0;
          if (forceSport && !GPSactive) gpsInit();
          flashRed(8);
        }
        seq = 0;
      }
    }
  }
  return accelMove;
}
