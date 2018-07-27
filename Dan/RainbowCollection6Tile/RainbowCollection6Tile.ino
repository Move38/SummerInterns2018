byte gameState;
byte colorSetup;
byte faceColors[6];
Color colorList[] = {RED, ORANGE, YELLOW, GREEN, BLUE, MAGENTA};
Timer victoryAnimTimer;

void setup() {
  // put your setup code here, to run once:
  gameState = 0;
  colorSetup = 0;
  cycleColors(0);
  setAllColors();
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (gameState) {
    case 0:
      setupPhaseLoop();
      break;
    case 1:
      break;
    case 2:
      gamePhaseLoop();
      break;
    case 3:
      victoryPhaseLoop();
      break;
  }

  //you can reset any blink by triple clicking
  if (buttonMultiClicked() == true && buttonClickCount() >= 3) { //so theoretically I can use this as a reset function
    setup();
  }
}

void setupPhaseLoop() { //Here we do the logic during setup phase
  if (buttonSingleClicked()) {
    //we transition through colors here
    colorSetup += 1;
    if (colorSetup > 5) {
      colorSetup -= 6;
    }
    cycleColors(colorSetup);
  } else if (buttonDoubleClicked()) {
    //we transition to game state, keeping our colors
    gameState = 2;
  }
}

void cycleColors(byte col) {
  FOREACH_FACE(f) {
    faceColors[f] = col;
  }
  setAllColors();
}

void gamePhaseLoop() {
  if (buttonDown()) {
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {
        faceColors[f] = getLastValueReceivedOnFace(f);
        setAllColors();
        victoryCheck();
      }
    }
  }
}

void setAllColors() { //This is the function we use to update color and values
  FOREACH_FACE(f) {
    //this is where we actually set the color physically
    setFaceColor(f, colorList[faceColors[f]]);
    setValueSentOnFace(faceColors[f], f);
  }
}

void victoryCheck() {
  //so in here we check if we have 6 different colors
  bool colorChecklist[6];
  bool victoryBool = true;
  //reset the checklist
  for (int j = 0; j < 6; j++) {
    colorChecklist[j] = false;
  }

  //remake the checklist
  FOREACH_FACE(f) {
    //grab the value of the data on that face, set to true at that position in the checklist
    colorChecklist[faceColors[f]] = true;
  }

  //now run through the checklist and see if there are any falses
  for (int i = 0; i < 6; i++) {
    if (colorChecklist[i] == false) {
      victoryBool = false;
    }
  }//if the bool is still true at the end of this, victory has been achieved!

  if (victoryBool) {
    gameState = 3;
    victoryAnimTimer.set(100);
  }
}

void victoryPhaseLoop() {
  if (victoryAnimTimer.isExpired()) {
    //in here we shuffle the colors along
    for (int i = 0; i < 6; i++) {
      faceColors[i] += 1;
      if (faceColors[i] == 6) {
        //this means we've gone past the last color and need to go to 0
        faceColors[i] = 0;
      }
    }
    setAllColors();
    victoryAnimTimer.set(100);
  }
}
