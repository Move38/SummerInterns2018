Color colors[] = {RED, ORANGE, CYAN, GREEN, BLUE, MAGENTA };

#include "Serial.h"

//this is the documentation for Honey comb

ServicePortSerial Serial;

//-- base variables
int count = 0;
byte busyBee = 0;
int stage;  //variable in case game can be expanded in the future to multiple 'stages'

int redNo = 20;
int orangeNo = 35;
int cyanNo = 60;
int greenNo = 100;
int blueNo = 200;
int magentaNo = 260;
int stage2No = 350;

byte currentColorIndex = 0;

Color baseColor;

Timer flashTimer; //timers for the flashing upgrade animation
bool didUpgradeHappen = false;

#define GOLDEN makeColorRGB(232, 190, 53) //stage 1 victory state color

bool oUpgrade = false;
bool cUpgrade = false;
bool gUpgrade = false;
bool bUpgrade = false;
bool mUpgrade = false;

int upValue = 1; //base value, cyan & blue upgrades this value

// -- orange/green upgrade variables
Timer oTimer;
int oTimerNumber = 4000;
int timerBonus = 1;
byte oUpgradeState = 0;
byte anyUpgradeState = 0;


enum State { //enum to make oTimer more functional
  ENABLED,
  DISABLED
};

//-- magenta upgrade variables
Timer mTimer;
Timer cdTimer;

void setup() {
  // put your setup code here, to run once:
  Serial.begin();
  baseColor = WHITE;
  count = 0;
  flashTimer.set(500);
}


void loop() {

  FOREACH_FACE(f) {
    getLastValueReceivedOnFace(f);
  }

  setColor(baseColor); //base color

  if (buttonPressed()) { //basic click mechanism, add to count
    count += upValue;
    busyBee += 1;
    Serial.println(count);
  }

  if (buttonLongPressed()) { //reset count and face colors if longpressed
    reset();
  }

  //-- orange upgrade periodically adds to count automatically

  if (oUpgrade == true && oUpgradeState == ENABLED)  {
    Serial.println("timer set!");
    oTimer.set(oTimerNumber);
    oUpgradeState = DISABLED;

  }

  if (gUpgrade == true) {
    oTimerNumber = 2000; //green shortens the time for the oTimer
  }

  if (mUpgrade == true) {
    oTimerNumber = 500;
  } //magenta shortens it further

  if (oTimer.isExpired() && oUpgrade == true) {
    count += timerBonus;
    Serial.println(count);
    if (oUpgrade == true) {
      oTimer.set(oTimerNumber);
      Serial.println("timer reset!");
      busyBee += 1;
    }

  }

  //-- yellow upgrade doubles your click value to 2

  if (cUpgrade == true) {
    upValue = 2 ;
  }

  if (bUpgrade == true) { //blue enhances it to 3
    upValue = 3;
  }

  //-- below section changes a face color depending on count!

  if (didUpgradeHappen == true && anyUpgradeState == ENABLED) { //flash upgrade color
    flashTimer.set(2000);
    anyUpgradeState = DISABLED;
    setColor(colors[currentColorIndex]);
    if (flashTimer.isExpired()) {
      didUpgradeHappen = false;
    }
    Serial.println("upgrade happened!");

  }

  if (count > redNo) {
    currentColorIndex++;
    setFaceColor(0, RED);
    setValueSentOnFace(0, 0);

  }

  if (count > orangeNo) {
    currentColorIndex++;
    setFaceColor(1, ORANGE );
    setValueSentOnFace(1, 1);

  }

  if (count > cyanNo) {
    currentColorIndex++;
    setFaceColor(2, CYAN);
    setValueSentOnFace(2, 2);

  }

  if (count > greenNo) {
    setFaceColor(3, GREEN);
    setValueSentOnFace(3, 3);

  }

  if (count > blueNo) {
    currentColorIndex++;
    setFaceColor(4, BLUE);
    setValueSentOnFace(4, 4);

  }

  if (count > magentaNo ) {
    currentColorIndex++;
    setFaceColor(5, MAGENTA);
    setValueSentOnFace(5, 5);

  }

  if (count > stage2No ) {
    baseColor = GOLDEN;
    setColor(baseColor);

  }

  //--below section enables upgrades once count is a certain number!

  if (getLastValueReceivedOnFace(1) == 1) {
    didUpgradeHappen = true;
    oUpgrade = true;
    if (didValueOnFaceChange(1)) {
      Serial.println("oUpgrade enabled");
    }
  }

  if (getLastValueReceivedOnFace(2) == 2) {
    didUpgradeHappen = true;
    cUpgrade = true;
    if (didValueOnFaceChange(2)) {
      Serial.println("cUpgrade enabled");
    }

  }

  if (getLastValueReceivedOnFace(3) == 3) {
    didUpgradeHappen = true;
    gUpgrade = true;
    if (didValueOnFaceChange(3)) {
      Serial.println("gUpgrade enabled");
    }
  }

  if (getLastValueReceivedOnFace(4) == 4) {
    didUpgradeHappen = true;
    bUpgrade = true;
    if (didValueOnFaceChange(4)) {
      Serial.println("bUpgrade enabled");
    }
  }

  if (getLastValueReceivedOnFace(5) == 5) {
    didUpgradeHappen = true;
    mUpgrade = true;
    if (didValueOnFaceChange(5)) {
      Serial.println("mUpgrade enabled");
    }
  }

  setFaceColor(busyBee, YELLOW);
  if (busyBee > 5) {
    busyBee = 0;
  }

} //end of loop

void reset() { //reset all variables
  Serial.println("reset");
  bool oUpgrade = false;
  bool cUpgrade = false;
  bool gUpgrade = false;
  bool bUpgrade = false;
  bool mUpgrade = false;
  setColor(WHITE);
  count = 0;
  int upValue = 1;
  int timerBonus = 5;
  oUpgradeState = 0;
  Serial.println(count);
  busyBee = 0;
  stage = 0;
  baseColor = WHITE;

}




