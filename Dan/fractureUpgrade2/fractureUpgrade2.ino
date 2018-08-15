byte gameState;
byte team;
Color playerColors[] = {ORANGE, YELLOW, GREEN, CYAN};
byte teamHues[] = {22, 42, 85, 128};

enum fractureStates {NOMINAL, FRACTURED, RESOLVING};

bool neighborStates[6];

//animation variables
bool isFlashing = false;
byte flashHue = 0;
Timer flashTimer;

void setup() {
  gameState = NOMINAL;
  setColor(makeColorHSB(teamHues[team], 255, 255));

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      neighborStates[f] = true;
    } else {
      neighborStates[f] = false;
    }
  }

  setFaceColor(0,RED);
}

void loop() {
  //first, we need to listen for double clicks to change team
  if (buttonDoubleClicked()) {
    team++;
    if (team > 3) {
      team = 0;
    }
    setColor(makeColorHSB(teamHues[team], 255, 255));
  }

  //determine which state I'm in, send to appropriate loop behavior
  switch (gameState) {
    case NOMINAL:
      nominalLoop();
      break;
    case FRACTURED:
      fracturedLoop();
      break;
    case RESOLVING:
      resolvingLoop();
      break;
  }

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      neighborStates[f] = true;
    } else {
      neighborStates[f] = false;
    }
  }

  setValueSentOnAllFaces((gameState * 10) + team);
}

void nominalLoop() {
  //check surroundings for MISSING NEIGHBORS or neighbors already in distress
  FOREACH_FACE(f) {
    if (isValueReceivedOnFaceExpired(f) && neighborStates[f] == true) { //missing neighbor
      gameState = FRACTURED;
    } else if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) / 10 == FRACTURED) { //distressed neighbor
      gameState = FRACTURED;
    }
  }

  if (gameState == FRACTURED) {
    //begin the red flash, change the state
    isFlashing = true;
    flashHue = 0;
    setColor(makeColorHSB(flashHue, 255, 255));
    flashTimer.set(200);
  }
}

void fracturedLoop() {
  //first we do the flash animation
  if (isFlashing) {

  }


  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && neighborStates[f] == false) { //new neighbor
      gameState = RESOLVING;
    } else if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) / 10 == RESOLVING) { //resolving neighbor
      gameState = RESOLVING;
    }
  }

  if (gameState == RESOLVING) {
    setColor(WHITE);
  }
}

void resolvingLoop() {
  gameState = NOMINAL;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) / 10 == FRACTURED) {
      gameState = RESOLVING;
    }
  }

  if (gameState == NOMINAL) {
    setColor(makeColorHSB(teamHues[team], 255, 255));
  }
}

