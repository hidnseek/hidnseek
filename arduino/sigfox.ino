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

void initSigFox() {
  DEBUG_PRINTLN("Init SigFox");
  delay(1000);
  if (Akeru.begin() != 3) {
    ModemPresent = false;
    error("Modem not responding");
  } 
  else {
    ModemPresent = true;
    delay(2000);
  }
}

void sendSigFox() {
  if (Akeru.isReady()) { // Send in the mighty internet!
    takeState = true;
    stepMsg();
    if (MsgCount < MsgMax) {
      Akeru.send(&p,sizeof(p));
      #ifdef DEBUG
        Serial.print("Akeru Send ");
        Serial.print(sizeof(p));
        Serial.println(" octets");
      #endif
      EEPROM.write(gpsDay, ++MsgCount);
    }
    akeruTime = 0;
  }
}

