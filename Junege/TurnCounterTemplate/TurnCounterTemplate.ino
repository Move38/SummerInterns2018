byte gameState;
byte playerCount = 4; //how many players there are
byte currentPlayer; //whose turn it is
byte totalMoves = 0;

//color index to represent four players. For testing purposes the player count is 4; future games will need an input mechanism to determine a variable amount of players
Color playerColors[] = {RED, YELLOW, ORANGE, MAGENTA};

bool neighborStates[6]; //6 faces to each Blink

enum State { //three states for each of the player's moves
  NOMINAL, //0
  FRACTURED, //1
  RESOLVING, //2
};

void setup() {
  totalMoves = 0;
  gameState = NOMINAL;
  setColor(OFF);
  setTurnIndicator ();
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      neighborStates[f] = true;
    } else {
      neighborStates[f] = false;
    }
  }
}

void loop() {

  if (buttonDoubleClicked()) { //this is to reset a Blink to the default behavior, only used for testing/debugging purposes
    setup();
  }

  //determine which state Blink is in, send to appropriate loop behavior
  switch (gameState) {
    case NOMINAL: //this state is activated when all Blinks are together
      nominalLoop();

      break;

    case FRACTURED: //this state is when one or more Blink(s) are separated from the rest of the Blinks
      fracturedLoop();

      break;

    case RESOLVING: //a resolution state that transitions to NOMINAL immediately; to prevent the two other case states from interfering with each othe r
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

  setValueSentOnAllFaces(gameState); //relay our game state to our neighbors 

}

void nominalLoop () {
  //check surroundings for MISSING NEIGHBORS or neighbors already in distress
  FOREACH_FACE(f) {
    if (isValueReceivedOnFaceExpired(f) && neighborStates[f] == true) { //missing neighbor
      gameState = FRACTURED;
    } else if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == FRACTURED) { //detecting a distressed neighbor
      gameState = FRACTURED;
    }
  }

  if (gameState == FRACTURED) { //turn WHITE to indicate fractured state 
    setColor(WHITE);
    setTurnIndicator ();
  }
}

void fracturedLoop () {

  //check surroundings for NEW NEIGHBORS or neighbors in the resolution state 
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && neighborStates[f] == false) { //new neighbor
      gameState = RESOLVING;
    } else if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == RESOLVING) { //next to a resolved neighbor
      gameState = RESOLVING;
    }
  }

  if (gameState == RESOLVING) { //no color to indicate resolution state 
    setColor(OFF);
    setTurnIndicator ();
  }

}

void resolvingLoop () {
  gameState = NOMINAL;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == FRACTURED) {
      gameState = RESOLVING;
    }
  }

  if (gameState == NOMINAL) {
    setColor(OFF);
    totalMoves++; //this is where we increment the total turn count 
    setTurnIndicator ();

  }

}

void setTurnIndicator () {
  currentPlayer = totalMoves % playerCount; //we indicate a player's turn based on the movecount and the number of players
  setFaceColor(0, playerColors[currentPlayer]); //each player is represented by a different color. The 0 face color indicates the player's turn

}



