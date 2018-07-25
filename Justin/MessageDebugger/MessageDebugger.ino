/*
   Message Debugger
*/

#include "Serial.h"

ServicePortSerial Serial;

void setup() {
  // put your setup code here, to run once:
  Serial.begin();
}

void loop() {
  // put your main code here, to run repeatedly:

  // read in message on each face
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      if (didValueOnFaceChange(f)) {
        Serial.print("face ");
        Serial.print(f);
        Serial.print(": ");
        Serial.println(getLastValueReceivedOnFace(f));
      }
    }
  }
}
