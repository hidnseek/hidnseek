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

void gpsCmd (PGM_P s) {
  int XOR = 0;
  char c;
  while ((c = pgm_read_byte(s++)) != 0) {
    Serial.print(c);
    if (c == '*') break;
    if (c != '$') XOR ^= c;
  }
  if (XOR < 0x10) Serial.print("0");
  Serial.println(XOR, HEX);
}

bool gpsInit()
{
  boolean GPSready = false;
  digitalWrite(rstPin, HIGH);
  unsigned long startloop = millis();
  while ((uint16_t) (millis() - startloop) < 6000 ) {
    if (Serial.available() > 0 && Serial.read() == '*') {
      GPSready = true;
      break;
    }
    delay(100);
  }
  if (GPSready) {
    gpsCmd(PSTR(PMTK_SET_NMEA_OUTPUT_RMCGGA));
    gpsCmd(PSTR(PMTK_SET_NMEA_UPDATE_1HZ));   // 1 Hz update rate
  } else digitalWrite(rstPin, LOW);
  return GPSready;
}

void gpsStandby() {
  GPSactive = false;
  digitalWrite(rstPin, LOW);
}

bool gpsProcess()
{
  boolean newGpsData = false;
  boolean newSerialData = false;
  float distance;
  unsigned long start = millis();
  unsigned int waitime = 2000;
  // Parse GPS data for 2 second
  while ((uint16_t) (millis() - start) < waitime)
  {
    if (Serial.available() > 0) {
      newSerialData = true;
      waitime = 100;
      start = millis();
      redLEDon;
    }
    while (Serial.available())
    {
      char c = Serial.read();
      // New valid NMEA data available
      if (gps.encode(c)) newGpsData = true;
    }
  }

  // Check if NMEA packet received, wake up GPS otherwise
  if (!newSerialData) gpsInit();

  // 12 octets = 96 bits payload
  // lat: 32, lon: 32, alt: 13 , spd: 7, cap: 2, bat: 7, mode: 3 (0-3 sat view, more is MSG)
  // lat: 32, lon: 32, alt:0-8191m, spd:0-127Km/h, bat:0-100%, mode:0-7, cap: N/E/S/W
  // int is 16 bits, float is 32 bits. All little endian

  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);

  if (newGpsData) { // computeData
    gps.f_get_position(&p.lat, &p.lon, &fix_age);
    sat = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();
    alt = abs(round(gps.f_altitude()));
    spd = round(gps.f_speed_kmph());
    syncSat += sat;
    noSat = 0;
  }
  else noSat++;

  //printData(newGpsData); // For debug purpose

  redLEDoff;
  return newSerialData;
}

void print_date()
{
  char sz[24];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d ",
          month, day, year, hour, minute, second);
  Serial.print(sz);
}

void printData(bool complete) {
  print_date();
  if (complete) {
    serialString(PSTR("syncSat="));
    Serial.print(syncSat);
    serialString(PSTR(", lat="));
    Serial.print(p.lat, 7);
    serialString(PSTR(", lon="));
    Serial.print(p.lon, 7);
    serialString(PSTR(", alt="));
    Serial.print(alt);
    serialString(PSTR(", cap="));
    Serial.print((gps.course() / 90) % 4);
    serialString(PSTR(", spd="));
    Serial.print(spd);
    serialString(PSTR(", sat="));
    Serial.print(sat);
  } else {
    serialString(PSTR(" noSat="));
    Serial.print(noSat);
  }
  if (forceSport) {
    serialString(PSTR(", sport"));
  }
  serialString(PSTR(", bat="));
  Serial.println(batteryPercent);
  Serial.flush();
}

void makePayload() {
  if (forceSport) {
    if ( previous_lat == 0 && previous_lon == 0) {
      previous_lat = p.lat;
      previous_lon = p.lon;
    }
    if (gps.distance_between(p.lat, p.lon, previous_lat, previous_lon) < DOG_DISTANCE) syncSat = 255;
  }
  uint8_t cap;
  if (syncSat != 0) {
    if (spd > 120 && alt > 1250) {
      airPlaneSpeed = true;  // Abort GPS message if Airplane detected
      syncSat = 255;
    }
    if (spd < 120) airPlaneSpeed = false;

    if (alt > 4096) alt = (uint16_t)(alt / 16) + 3840; // 16m step after 4096m
    if (alt > 8191) alt = 8191;                        // 69632m is the new limit ;)

    if (spd > 127) spd = (uint16_t)(spd / 16) + 94; // 16Km/h step after 127Km/h
    else if (spd > 90) spd = (uint16_t)(spd / 3) + 60; // 3Km/h step after 90Km/h
    if (spd > 126) spd = 127;      // limit is 528Km/h
    cap = (gps.course() / 90) % 4;
  } else cap = (accelPosition < 3) ? accelPosition : 3;

  p.cpx = (uint32_t) alt << 19;
  p.cpx |= (uint32_t) spd << 12; // send in Km/h
  p.cpx |= (uint32_t) cap << 10;  // send N/E/S/W
  p.cpx |= (uint32_t) ( 127 & batteryPercent) << 3; // bat (7bits)
  if (sat > 8) sat = 8;
  p.cpx |= (uint32_t) 3 & (sat / 4); // sat range is 0 to 14
}

void decodPayload() {
  unsigned int alt_ = p.cpx >> 19;
  unsigned int spd_ = (p.cpx >> 12) & 127;
  unsigned int cap_ = (p.cpx >> 10) & 3;
  unsigned int bat_ = (p.cpx >> 3) & 127;
  unsigned int mod_ = p.cpx & 7;
  print_date();
  serialString(PSTR("msg="));
  Serial.print(MsgCount);
  serialString(PSTR(" lat="));
  Serial.print(p.lat, 7);
  serialString(PSTR(", lon="));
  Serial.print(p.lon, 7);
  serialString(PSTR(", alt="));
  Serial.print(alt_);
  serialString(PSTR(", cap="));
  Serial.print(cap_);
  serialString(PSTR(", spd="));
  Serial.print(spd_);
  serialString(PSTR(", bat="));
  Serial.print(bat_);
  serialString(PSTR(", mode="));
  Serial.println(mod_);
}
