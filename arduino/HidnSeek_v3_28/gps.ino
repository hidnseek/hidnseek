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
  long startloop = millis();
  while (millis() - startloop < 6000 ) {
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
      waitime = 200;
      start = millis();
      digitalWrite(redLEDpin, HIGH);
    }
    while (Serial.available())
    {
      char c = Serial.read();
      // New valid NMEA data available
      if (gps.encode(c))
      {
        newGpsData = true;
      }
    }
    digitalWrite(redLEDpin, LOW);
  }

  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);

  if (newGpsData) { // computeData
    gps.f_get_position(&p.lat, &p.lon, &fix_age);
    serialString(PSTR("lat="));
    Serial.print(p.lat, 7);
    serialString(PSTR(", lon="));
    Serial.print(p.lon, 7);
    serialString(PSTR(", syncSat="));
    Serial.print(syncSat);
    serialString(PSTR(", fix age="));
    Serial.println(fix_age);
    distance = 1000;
    if (fix_age == TinyGPS::GPS_INVALID_AGE || fix_age > 5000) {
      newGpsData = false; // No fix detected
      p.lat = previous_lat;
      p.lon = previous_lon;
      serialString(PSTR("recover lat="));
      Serial.print(p.lat, 7);
      serialString(PSTR(", lon="));
      Serial.println(p.lon, 7);
    } else if (abs(p.lat) > 2 && abs(p.lon) > 2) distance = gps.distance_between(p.lat, p.lon, previous_lat, previous_lon);
    if (newGpsData && distance < 25 && syncSat > 20) {
      syncSat = 255;
    }

    unsigned int alt = abs(round(gps.f_altitude()));
    if (alt > 8191) alt = 8191;
    p.cpx = (uint32_t) alt << 19;
    unsigned int spd = round(gps.f_speed_kmph());
    if (spd > 90) spd = (int)(spd / 3) + 60;
    if (spd > 126) spd = 127;
    p.cpx |= (uint32_t) spd << 12; // send in Km/h
    p.cpx |= (uint32_t) ( (gps.course() / 90) % 4) << 10;  // send N/E/S/W
    p.cpx |= (uint32_t) ( 127 & batteryPercent) << 3; // bat (7bits)
    int sat = gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites();
    p.cpx |= (uint32_t) ( 3 & int(sat / 4)); // sat range is 0 to 14
    if (newGpsData) {
      if (sat > 3 && abs(p.lat) > 2 && abs(p.lon) > 2) {
        noSat = 0;
        syncSat++; // else syncSat = 0; // increase global variable
      }
      else noSat++;
      if ((gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop()) < 100) syncSat ++;
    }
    else noSat++;
  }
  else noSat++;

  print_date();
  serialString(PSTR("sat="));
  Serial.print(gps.satellites());
  serialString(PSTR(", noSat="));
  Serial.print(noSat);
  serialString(PSTR(", syncSat="));
  Serial.println(syncSat);

  return newGpsData;
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
}

void print_date()
{
  char sz[32];
  sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d   ",
          month, day, year, hour, minute, second);
  Serial.print(sz);
  print_int(fix_age, TinyGPS::GPS_INVALID_AGE, 5);
}
