/*
 * MMA845XQ test code
 * (C) 2012 Akafugu Corporation
 *
 * This program is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 */


#include "Wire.h"
#include "MMA8653.h"

MMA8653 accel;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  accel.begin(false, 2);
  pinMode (6, OUTPUT);
  pinMode (9, OUTPUT);
  pinMode (7, OUTPUT);
  digitalWrite(9, HIGH);
  digitalWrite(7, LOW);
}


uint8_t rdata;
uint8_t i;
int x, x1, y, y1, z, z1;
uint8_t stat;
String stringOutput;
long start;

#define TRG 120

void loop() {
  accel.update();
  //Serial.println("X,\tY,\tZ,\tRho,\tPhi,\tTheta");
  if (abs(accel.getX()) > TRG || abs(accel.getY()) > TRG || abs(accel.getZ()) > TRG) digitalWrite(6, HIGH); else digitalWrite(6, LOW);
  if ((millis() - start > 1000)) {
    start = millis();
    Serial.print(" , ");
    Serial.print(accel.getX());
    Serial.print(" , ");
    Serial.print(accel.getY());
    Serial.print(", ");
    Serial.print(accel.getZ());
    Serial.print(", ");
    Serial.print(accel.getRho());
    Serial.print(", ");
    Serial.print(accel.getPhi());
    Serial.print(", ");
    Serial.print(accel.getTheta());
    Serial.println();
  }
}
