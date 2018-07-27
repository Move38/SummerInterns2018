byte gameState;
/*
   Game states are:
   0 - setup
   1 - waiting for all color connections (only experienced by the center piece
   2 - active play (center)
   3 - active play (dials)
*/
bool isCenter;
Color dialColor = WHITE;

byte currentR = 0;
byte currentG = 0;
byte currentB = 0;

byte fStops[] = {8, 16, 32, 64, 128, 255};

void setup() {
  // put your setup code here, to run once:
  gameState = 0;
  isCenter = false;
  FOREACH_FACE(f) {
    //set the dimming lights around the edge (turning out to be difficult, not necessary at this step)
    setFaceColor(f, makeColorRGB(fStops[f], fStops[f], fStops[f]));
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (gameState) {
    case 0:
      setupLoop();
      break;
    case 1:
      transitionLoop();
      break;
    case 2:
      gameLoop();
      break;
    case 3:
      //YOU DO NOTHING, YOU'RE A DIAL
      break;
  }

}

void setupLoop() {
  if (buttonDoubleClicked()) {
    //this means you are the center
    setColor(OFF);
    setFaceColor(0, RED);
    setValueSentOnFace(1, 0);
    setFaceColor(2, GREEN);
    setValueSentOnFace(2, 2);
    setFaceColor(4, BLUE);
    setValueSentOnFace(3, 4);
    gameState = 1;
  }

  if (buttonMultiClicked() == true && buttonClickCount() >= 3) { //so theoretically I can use this as a reset function
    //you are being set back to an outside thing. mostly for testing
    setup();
  }

  //mostly, you wait to hear from a neighbor that you have been assigned a color
  FOREACH_FACE(f) {
    if (getLastValueReceivedOnFace(f) == 1 && !isValueReceivedOnFaceExpired(f)) {
      //you have been commanded to be RED
      currentR = 1;
    } else if (getLastValueReceivedOnFace(f) == 2 && !isValueReceivedOnFaceExpired(f)) {
      currentG = 1;
    } else if (getLastValueReceivedOnFace(f) == 3 && !isValueReceivedOnFaceExpired(f)) {
      currentB = 1;
    }
  }

  //we've finished checking for dial color. If we found one, do the thing
  if (currentR + currentG + currentB == 1) { //this means one color has been set to 1
    FOREACH_FACE(f) {
      setFaceColor(f, makeColorRGB(fStops[f]*currentR, fStops[f]*currentG, fStops[f]*currentB));
      setValueSentOnFace(f, f);
    }
    gameState = 3;
  }
}

void transitionLoop() {
  //this is only experienced by the center piece
  if (!isValueReceivedOnFaceExpired(0) && !isValueReceivedOnFaceExpired(2) && !isValueReceivedOnFaceExpired(4)) {
    //great, you have all three dials. Your output stays the same, but the display changes DRASTICALLY
    gameState = 2;
  }
}

void gameLoop() {
  //this code is only  experienced by the center piece
  if (!isValueReceivedOnFaceExpired(0)) {
    currentR = fStops[getLastValueReceivedOnFace(0)];
  } else {
    currentR = 0;
  }

  if (!isValueReceivedOnFaceExpired(2)) {
    currentG = fStops[getLastValueReceivedOnFace(2)];
  } else {
    currentG = 0;
  }

  if (!isValueReceivedOnFaceExpired(4)) {
    currentB = fStops[getLastValueReceivedOnFace(4)];
  } else {
    currentB = 0;
  }

  setColor(makeColorRGB(currentR, currentG, currentB));
}

