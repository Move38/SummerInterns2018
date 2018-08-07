/*
    Template for creating a game that has a move action that needs to be communicated across
    the entire board. i.e. Each move a player makes causes a stepwise reaction

    TODO: If a single move is missed, the counters will get out of sync, one possible sol'n
    is to share the turn count and make sure that you are up to date with the highest. This
    of course does not allow a Blink to understand the context of the previous moves, so
    catching up in the game may be useful or not depending on the needs. If the game simply
    needs to end after 40 moves, then catching up the game counter is a worthy sol'n.
*/

byte gameState;
byte playerCount = 4; //how many players there are
byte currentPlayer; //whose turn it is
byte totalMoves = 0;

enum State { //three states for each of the player's moves
  MOVE_IDLE,          // 0
  MOVE_IN_PROGRESS,   // 1
  MOVE_RESOLVING      // 2
};

void setup() {
  // INIT variables
  totalMoves = 0;
  gameState = MOVE_IDLE;
}

/*###################
        BEGIN LOOP
  ###################*/
void loop() {

  if (isAlone()) {
    setup();  // seperate Blinks to reset our turn counter (for demo purposes)
  }

  //determine which state Blink is in, send to appropriate loop behavior
  switch (gameState) {
    case MOVE_IDLE:         idleLoop();           break;
    case MOVE_IN_PROGRESS:  inProgressLoop();      break;
    case MOVE_RESOLVING:    resolvingLoop();       break;
  }

  // display game state
  switch (gameState) {
    case MOVE_IDLE:         displayIdle();        break;
    case MOVE_IN_PROGRESS:  displayInProgress();  break;
    case MOVE_RESOLVING:    displayResolving();  break;
  }

  // communicate our state to others
  setValueSentOnAllFaces(gameState); //relay our game state to our neighbors

}
/*###################
        END LOOP
  ###################*/



/*------------------------------------------------------------
                    MOVE STATE LOGIC
  ------------------------------------------------------------*/
void idleLoop () {

  // check surroundings for neighbor in progress
  FOREACH_FACE(f) {
    // if a neighboring piece is move in progress, then we should be as well
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == MOVE_IN_PROGRESS) {
      gameState = MOVE_IN_PROGRESS;
    }
  }

  /**********************************************************
           [BEGIN] REPLACE THIS WITH YOUR TURN MECHANISM
   **********************************************************/
  if (buttonPressed()) {
    gameState = MOVE_IN_PROGRESS;
  }
  /**********************************************************
           [END] REPLACE THIS WITH YOUR TURN MECHANISM
   **********************************************************/
}

void inProgressLoop () {

  // if a neighboring piece is in resolution, then we should be as well
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == MOVE_RESOLVING) {
      gameState = MOVE_RESOLVING;
    }
  }

  /**********************************************************
           [BEGIN] REPLACE THIS WITH YOUR TURN MECHANISM
   **********************************************************/
  if (buttonReleased()) {
    gameState = MOVE_RESOLVING;
  }
  /**********************************************************
           [END] REPLACE THIS WITH YOUR TURN MECHANISM
   **********************************************************/
}

void resolvingLoop () {
  gameState = MOVE_IDLE;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == MOVE_IN_PROGRESS) {
      gameState = MOVE_RESOLVING;
    }
  }

  if (gameState == MOVE_IDLE) {
    totalMoves++; //this is where we increment the total turn count
  }

}


/*------------------------------------------------------------
                    DISPLAY LOOPS
  ------------------------------------------------------------*/
void displayIdle() {
  setColor(OFF);
  setTurnIndicator ();
}

void displayInProgress() {
  setColor(WHITE);
  setTurnIndicator ();
}

void displayResolving() {
  setColor(OFF);
  setTurnIndicator ();
}

/*------------------------------------------------------------
                  Get the Current Player
  ------------------------------------------------------------*/
byte getCurrentPlayer() {
  return totalMoves % playerCount;  //we indicate a player's turn based on the movecount and the number of players
}

/*------------------------------------------------------------
              Light up one pixel to show Player
  ------------------------------------------------------------*/
void setTurnIndicator () {
  byte currentPlayer = getCurrentPlayer();
  setFaceColor(0, getColorForPlayer(currentPlayer)); //each player is represented by a different color. The 0 face color indicates the player's turn
}

/*------------------------------------------------------------
              Gets a unique color for each player
  ------------------------------------------------------------*/
Color getColorForPlayer( byte playerIndex ) {

  Color c;

  switch (playerIndex) {
    case 0: c = RED;      break;
    case 1: c = YELLOW;   break;
    case 2: c = ORANGE;   break;
    case 3: c = MAGENTA;  break;
    case 4: c = BLUE;     break;
    case 5: c = GREEN;    break;
    default: c = OFF;     break;
  }

  return c;
}


