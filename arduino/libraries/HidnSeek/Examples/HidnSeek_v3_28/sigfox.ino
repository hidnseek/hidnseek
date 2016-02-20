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

bool initSigFox() {
  serialString(PSTR("SigFox: "));
  unsigned long previousMillis = millis();
  while ((uint16_t) (millis() - previousMillis) < 6000) {
    if (HidnSeek.begin() == 3) {
      //Serial.println(millis() - previousMillis);
      Serial.println(HidnSeek.getID(), HEX);
      return true;
    }
    else delay(200);
  }
  Serial.println("Fail");
  return false;
}

void sendSigFox(byte msgType) {
  digitalWrite(redLEDpin, HIGH);
  // isReady check removed in the library due to reset of millis during sleep time
  if (msgType > 0) {
    if (baromPresent) {
      bmp180Measure(&Temp, &Press);
      if (Press > 1000) airPlaneSpeed = false;
    } else {
      Temp = 20;
      Press = 1030;
    }
    if (msgType == MSG_NO_MOTION) {
      p.lat = Temp;
      p.lon = Press;
    }
    p.cpx &= ~( 3 << 10); // orientation (2bits)
    p.cpx |= (uint32_t) ((accelPosition < 3) ? accelPosition : 3) << 10;  // send orientation
    p.cpx &= ~( 127 << 3); // bat (7bits)
    p.cpx |= (uint32_t) ( 127 & batteryPercent) << 3; // bat (7bits)
    p.cpx &= ~(7 << 0);
    p.cpx |= (uint32_t) (  7 & msgType); // mode (2bits)
  }
  else {
    makePayload();
    previous_lat = p.lat;
    previous_lon = p.lon;
  }
  decodPayload();
  unsigned long previousMillis = millis();
  if ( !(msgType > 0 && airPlanePress) && !airPlaneSpeed) {
    HidnSeek.send(&p, sizeof(p));
    stepMsg(); // Init the message number per day and time usage counters
    while ((uint16_t) (millis() - previousMillis) < 6000) delay(100);
  }
  digitalWrite(redLEDpin, LOW);
}
