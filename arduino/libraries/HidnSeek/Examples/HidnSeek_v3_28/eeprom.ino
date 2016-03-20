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
        EEPROM.write(day, 0);
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
  if (today == 255) {
    today = 0;
    EEPROM.write(ADDR_TODAY, 0);;
  }
  serialString(PSTR("Last day:"));
  Serial.println(today);
  // Display numbers of messages sent to sigfox network on day 1, 2, 3, ... , 30, 31
  serialString(PSTR("Usage per days:"));
  for (int i = ADDR_SENT; i < ADDR_SENT + 31; i++) {
    MsgCount = EEPROM.read(i);
    if (MsgCount == 255) {
      EEPROM.write(i, 0);
      MsgCount = 0;
    }
    Serial.print(MsgCount);
    Serial.print(" ");
  }
  MsgCount = EEPROM.read(today);
  Serial.println("");
}
