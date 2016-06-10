/*  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  HidnSeek by StephaneD 20150313 Not for commercial use              */

unsigned int sensorMax;

void initSense() {
  byte lowByte = EEPROM.read(ADDR_CAL_LOW);
  byte highByte = EEPROM.read(ADDR_CAL_HIGH);
  sensorMax = ((lowByte << 0) & 0xFF) + ((highByte << 8) & 0xFF00);
  serialString(PSTR("SensorMax: "));
  Serial.print(sensorMax);
  sensorMax = 980;
}

unsigned int calibrate(unsigned int sensorValue) {
  // record the maximum sensor value
  byte lowByte = ((sensorValue >> 0) & 0xFF);
  byte highByte = ((sensorValue >> 8) & 0xFF);
  EEPROM.write(ADDR_CAL_LOW, lowByte);
  EEPROM.write(ADDR_CAL_HIGH, highByte);
  return sensorValue;
}

#define NUM_READS 100
bool batterySense() {
  digitalWrite(rstPin, HIGH);
  analogReference(EXTERNAL);
  delay(100); // RC need 30ms
  // read multiple values and sort them to take the median value. Require 24ms
  uint8_t sortedValues[NUM_READS];
  for (uint8_t i = 0; i < NUM_READS; i++) {
    uint8_t value = analogRead(sensorBatt) >> 2;
    uint8_t j;
    if (value < sortedValues[0] || i == 0) {
      j = 0; //insert at first position
    }
    else {
      for (j = 1; j < i; j++) {
        if (sortedValues[j - 1] <= value && sortedValues[j] >= value) {
          // j is insert position
          break;
        }
      }
    }
    for (uint8_t k = i; k > j; k--) {
      // move all values higher than current reading up one position
      sortedValues[k] = sortedValues[k - 1];
    }
    sortedValues[j] = value; //insert current reading
  }
  batteryValue = 0;
  //return scaled mode of 3 values
  for (uint8_t i = NUM_READS / 2 - 4; i < (NUM_READS / 2 + 4); i++) {
    batteryValue += sortedValues[i];
  }
  batteryValue = batteryValue >> 1;

  unsigned int batteryComp = batteryValue - 4; // remove 16mV
  if (batteryComp > sensorMax) sensorMax = calibrate(batteryComp);
  unsigned int bat = map(batteryValue, 0, sensorMax, 0, BATT_MAX); // represent the battery voltage
  batteryPercent = map(bat, min(bat, BATT_MIN), max(bat, BATT_MAX), 0, 100);
  digitalWrite(rstPin, (forceSport || GPSactive) ? HIGH : LOW);
  return (bat < BATT_MIN);
}

void shutdownSys() { // 3.57V on battery voltage
  digitalWrite(rstPin, LOW);
  serialString(PSTR("Low Bat: "));
  saveEEprom();
  sendSigFox(MSG_WEAK_BAT);
  digitalWrite(shdPin, LOW);
  delay(500);
}
