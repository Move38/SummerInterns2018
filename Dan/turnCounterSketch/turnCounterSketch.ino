byte playerCount = 4;//never changes
word totalMoves = 0;//increments each turn
byte currentPlayer = 0;//changes from 0-3 as turns go by

byte gameState;

enum fractureStates {NOMINAL, FRACTURED, RESOLVING};
//                    0         1          2

Color playerColors[] = {RED, YELLOW, ORANGE, MAGENTA};

bool neighborStates[6];

void setup() {
  gameState = NOMINAL;
  setColor(OFF);
  totalMoves = 0;

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      neighborStates[f] = true;
    } else {
      neighborStates[f] = false;
    }
  }
}

void loop() {
  if (buttonDoubleClicked()) {
    setup();
    setTurnIndicator();
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

  setValueSentOnAllFaces(gameState);
}

void nominalLoop() {
  //check surroundings for MISSING NEIGHBORS or neighbors already in distress
  FOREACH_FACE(f) {
    if (isValueReceivedOnFaceExpired(f) && neighborStates[f] == true) { //missing neighbor
      gameState = FRACTURED;
    } else if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == FRACTURED) { //distressed neighbor
      gameState = FRACTURED;
    }
  }

  if (gameState == FRACTURED) {
    setColor(WHITE);
    setTurnIndicator();
  }
}

void fracturedLoop() {
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && neighborStates[f] == false) { //new neighbor
      gameState = RESOLVING;
    } else if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == RESOLVING) { //resolving neighbor
      gameState = RESOLVING;
    }
  }

  if (gameState == RESOLVING) {
    setColor(OFF);
    setTurnIndicator();
  }
}

void resolvingLoop() {
  gameState = NOMINAL;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == FRACTURED) {
      gameState = RESOLVING;
    }
  }

  if (gameState == NOMINAL) {
    setColor(OFF);
    totalMoves++;
    setTurnIndicator();
  }
}

void setTurnIndicator(){
  currentPlayer = totalMoves % playerCount;
  setFaceColor(0, playerColors[currentPlayer]);
}

