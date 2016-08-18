/*  This file is part of NazaCanDecoderSigfox.

  HidnSeek is free software: you can redistribute it and/or
  modify it under the terms of the GNU General Public License as published
  by the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  HidnSeek is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with HidnSeek.  If not, see <http://www.gnu.org/licenses/>.*/

// EEPROM map
// byte 0: today
// byte1-31: number of messages sent
// byte32: number of messages this hour
// byte33-39:cent/year/month/day/hour/min/sec of last message
// byte40-41:battery calibration

void stepMsg()
{
  if (day > 0) {
    if (day != today) { // Reset or new day
      today = EEPROM.read(ADDR_TODAY);
      if (today != day) { // New day
        saveEEprom();
        EEPROM.write(ADDR_TODAY, day);
        EEPROM.write(day, ADDR_TODAY);
        today = day;
        MsgCount = 0;
      }
    }
    MsgCount++;
  } else MsgCount = 0;
}

void saveEEprom() {
  if (today > 0 && today < 32) {
    EEPROM.write(today, MsgCount);
  }
}

void dumpEEprom() {
  // Display the last day used by the tracker
  today = EEPROM.read(ADDR_TODAY);
  if (today > 32) {
    today = 0;
    EEPROM.write(ADDR_TODAY, 0);;
  }
  serialString(PSTR("Last day:"));
  Serial.println(today);
  // Display numbers of messages sent to sigfox network on day 1, 2, 3, ... , 30, 31
  serialString(PSTR("Use per days:"));
  for (int i = ADDR_SENT; i < ADDR_SENT + 31; i++) {
    MsgCount = EEPROM.read(i);
    if (MsgCount == 255) {
      EEPROM.write(i, 0);
      MsgCount = 0;
    }
    Serial.print(MsgCount);
    Serial.print(" ");
  }
  MsgCount = EEPROM.read(ADDR_GPS_PTR);
  Serial.println(MsgCount);
  if (MsgCount > 89) EEPROM.write(ADDR_GPS_PTR, 0);
  MsgCount = EEPROM.read(today);
}

void checkRelease() {
  boolean isChanged = false;
  PORTD |= (1 << bluLEDpin);
  // Read the EEPROM firmware release
  uint8_t version_hard = EEPROM.read(ADDR_VER_HARD);
  uint8_t version_var  = EEPROM.read(ADDR_VER_VAR);
  uint8_t version_soft = EEPROM.read(ADDR_VER_SOFT);
  uint8_t version_prot = EEPROM.read(ADDR_VER_PROT);
  if (version_var != VERSION_VAR) {
    isChanged = true;
    EEPROM.write(ADDR_VER_VAR, VERSION_VAR);
  }
  if (version_soft != VERSION_SOFT) {
    isChanged = true;
    EEPROM.write(ADDR_VER_SOFT, VERSION_SOFT);
  }
  if (version_prot != VERSION_PROT) {
    isChanged = true;
    EEPROM.write(ADDR_VER_PROT, VERSION_PROT);
  }
  if (version_hard != VERSION_HARD) {
    isChanged = true;
    EEPROM.write(ADDR_VER_HARD, VERSION_HARD);
  }

  if (isChanged) {
    p.lat = (float) (VERSION_HARD) + (VERSION_VAR / 100.0);
    p.lon = (float) (VERSION_SOFT) + (VERSION_PROT / 100.0);
    p.cpx = (uint32_t) 3 << 10;  // firmware message
    p.cpx |= (uint32_t) ( 127 & batteryPercent) << 3; // bat (7bits)
    p.cpx |= (uint32_t) 3; // message type option

    serialString(PSTR("New firmware. "));
    delay(6000);
    unsigned long previousMillis = millis();
    HidnSeek.send(&p, sizeof(p),false);
    //Serial.println((uint16_t) (millis() - previousMillis));
    while ((uint16_t) (millis() - previousMillis) < 2000) delay(100);
  }
  PORTD &= ~(1 << bluLEDpin);
}

