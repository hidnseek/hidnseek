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
  along with HidnSeek.  If not, see <http://www.gnu.org/licenses/>

  This procedure store in eeprom GPS sent position to be replayed during the night
  Storage usage is
  lat 4 bytes
  lon 4 bytes
  spd 1 bytes    speed value is linear up to 90Km/h then x3 up to 127Km/h then x16
  tim 2 bytes    5s step time

  EEprom ADDR_GPS_DATA start at 45 up to 89 gps points this give 14 hours backup time

  Replay is done during no motion message until buffer is empty starting from most recent point.

*/

void gpsStore() {
  storePoint savePoint = {
    p.lat,
    p.lon,
    spd,
    (hour << 8) | (minute << 2) | (second >> 4)
  };
  //  (hour << 11) | (minute << 5) | (second >> 1)
  uint8_t lastAdress = EEPROM.read(ADDR_GPS_PTR);
  unsigned int eeAddress = ADDR_GPS_DATA + lastAdress * sizeof(savePoint);
  EEPROM.put(eeAddress, savePoint);
  if (lastAdress++ < ADDR_GPS_MAX) EEPROM.write(ADDR_GPS_PTR, lastAdress);
}

void makeReplay(uint8_t lastAdress_) {
  lastAdress_ = lastAdress_ - 1;
  storePoint savePoint;
  unsigned int eeAddress = ADDR_GPS_DATA + lastAdress_ * sizeof(savePoint);
  EEPROM.get(eeAddress, savePoint);
  EEPROM.write(ADDR_GPS_PTR, lastAdress_);
  p.lat = savePoint.lat;
  p.lon = savePoint.lon;
  spd = savePoint.spd;
  alt = savePoint.timer;
  //uint8_t _hour, _minute, _second;
  //_second = (savePoint.timer & 0x1f) << 1;
  //_minute = (savePoint.timer >> 5) & 0x3f;
  //_hour = (savePoint.timer >> 11);
  //alt = (uint16_t) ((_hour << 6 | _minute) << 2 | (_second >> 4));
}

void decodReplay() {
  byte _hour, _minute, _second;
  unsigned int timer = p.cpx >> 19;

  _hour   = timer >> 8;
  _minute = (timer & 0xff) >> 2;
  _second = (timer & 0x03) << 2;
  spd = (p.cpx >> 12) & 127;

  char sz[17];
  sprintf(sz, "replay:%02d:%02d:%02d ",
          _hour, _minute, _second);
  Serial.print(sz);

  Serial.print(p.lat, 7);
  serialString(PSTR(" "));
  Serial.print(p.lon, 7);
  serialString(PSTR(" "));
  Serial.print(spd);
  serialString(PSTR(" "));
  Serial.println(lastAdress);
}

void checkPV() {
  unsigned int PValue = analogReadSorted(sensorA0);
  //digitalWrite(chg500mA, (PValue > 128) ? HIGH : LOW);
  if (PValue > 128) PORTC |= (1 << 2); else PORTC &= ~(1 << 2);;
}

