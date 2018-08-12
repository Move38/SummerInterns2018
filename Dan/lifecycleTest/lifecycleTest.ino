enum blinkStates {
  SLEEP,
  WAKE,
  PROD,
  GAMEA,
  GAMEB,
  MENU,
  PROGRAMA,
  PROGRAMB,
  LEARNA,
  LEARNB,
  READY,
  YAWN,
  KO
};
byte blinkState;

enum menuStates {
  MENU_PROG,
  MENU_SLEEP
};
byte menuState = MENU_PROG;

byte currentGame = GAMEB;
byte programType = PROGRAMA;
bool programInitiator = false;

#define NUM_MENU_OPTIONS  2
#define WAKE_DELAY      100
#define WAKE_DURATION   300
#define SLEEP_DELAY     200
#define LEARN_DURATION 2000
#define PROG_DURATION  2000
#define READY_DURATION  500

#define MENU_TIMEOUT_DURATION    5000
#define SLEEP_TIMEOUT_DURATION  60000

Timer idleTimer;
Timer menuIdleTimer;
Timer progTimer;
Timer learnTimer;
Timer readyTimer;
Timer wakeDelayTimer;
Timer wakeDurationTimer;
Timer sleepDelayTimer;

void setup() {
  // put your setup code here, to run once:
  blinkState = WAKE;
  wakeDelayTimer.set(WAKE_DELAY);
  wakeDurationTimer.set(WAKE_DURATION);
  idleTimer.set(SLEEP_TIMEOUT_DURATION);
}

void loop() {
  // first, the loop switch
  switch (blinkState) {
    case SLEEP:     sleepLoop();    break;
    case WAKE:      wakeLoop();     break;
    case PROD:      wakeLoop();     break;
    case GAMEA:     gameLoop();     break;
    case GAMEB:     gameLoop();     break;
    case MENU:      menuLoop();     break;
    case PROGRAMA:  programLoop();  break;
    case PROGRAMB:  programLoop();  break;
    case LEARNA:    learnLoop();    break;
    case LEARNB:    learnLoop();    break;
    case READY:     readyLoop();    break;
    case YAWN:      yawnLoop();     break;
    case KO:        yawnLoop();     break;
  }

  //now some housekeeping, like making sure the sleep timer is reset after ANY interaction, and that you go to sleep if it expires
  if (buttonDown()) {
    idleTimer.set(SLEEP_TIMEOUT_DURATION);
  }

  // put idle blinks to sleep
  if (idleTimer.isExpired()) {
    blinkState = SLEEP;
  }

  // display states
  switch (blinkState) {
    case SLEEP:     displaySleep();    break;
    case WAKE:      displayWake();     break;
    case PROD:      displayProd();     break;
    case GAMEA:     displayGame();     break;
    case GAMEB:     displayGame();     break;
    case MENU:      displayMenu();     break;
    case PROGRAMA:  displayProgram();  break;
    case PROGRAMB:  displayProgram();  break;
    case LEARNA:    displayLearn();    break;
    case LEARNB:    displayLearn();    break;
    case READY:     displayReady();    break;
    case YAWN:      displayYawn();     break;
    case KO:        displayKO();       break;
  }

  setValueSentOnAllFaces(blinkState);
}

/*
    UPDATE LOOPS
*/

void sleepLoop() {
  if (buttonSingleClicked()) { //someone has hit the button. This is method 1 of waking up
    blinkState = WAKE;
    wakeDelayTimer.set(WAKE_DELAY);
    wakeDurationTimer.set(WAKE_DURATION);
  }
  // button cleanup
  buttonDoubleClicked();

  FOREACH_FACE(f) { //this looks for neighbors in WAKE state. Method 2 of waking up
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == PROD) {
      blinkState = WAKE;
      wakeDelayTimer.set(WAKE_DELAY);
      wakeDurationTimer.set(WAKE_DURATION);
      idleTimer.set(SLEEP_TIMEOUT_DURATION);
    }
  }
}

void wakeLoop() {
  // button cleanup
  buttonSingleClicked();
  buttonDoubleClicked();

  if (wakeDelayTimer.isExpired()) {
    blinkState = PROD;
    wakeDelayTimer.set(NEVER);
  }

  if (wakeDurationTimer.isExpired()) { //last frame of animation
    blinkState = currentGame;
    wakeDurationTimer.set(NEVER);
  }
}

/*
   Play game until one of the following is sensed
   1. Menu Action (alone and pressed for > n seconds)
   2. Neighbor is putting me to sleep (KO_STATE)
   3. Neighbor is programming me (PROGRAM_STATE)
*/
void gameLoop() {
  // button cleanup
  buttonSingleClicked();
  buttonDoubleClicked();

  //now we need to start looking out for menu press, but only when alone
  if (buttonMenuPressed()) {//you have to do this first to set the flag to false on every check
    if (isAlone()) {
      blinkState = MENU;
      menuIdleTimer.set(MENU_TIMEOUT_DURATION);
    }
  }

  //we need to look out for a KO or programming signals from neighbors
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//you found a neighbor

      byte neighbor = getLastValueReceivedOnFace(f);

      if (neighbor == KO) {                // Neighbor is puttin us to sleep

        blinkState = YAWN;
        sleepDelayTimer.set(SLEEP_DELAY);
      }
      else if (neighbor == PROGRAMA) {     // Neighbor is programming us game A

        blinkState = LEARNA;
        learnTimer.set(LEARN_DURATION);
      }
      else if (neighbor == PROGRAMB) {    // Neighbor is programming us game B

        blinkState = LEARNB;
        learnTimer.set(LEARN_DURATION);
      }
    }//end of found neighbor statement
  }//end of foreachface loop
}

void menuLoop() {
  if (buttonSingleClicked()) {
    menuState = ( menuState + 1 ) % NUM_MENU_OPTIONS;
    menuIdleTimer.set(MENU_TIMEOUT_DURATION);
  }

  if (buttonDoubleClicked()) { //type selected, move to appropriate mode

    menuIdleTimer.set(MENU_TIMEOUT_DURATION);

    if (menuState == MENU_PROG) {//head to the correct programming mode
      blinkState = programType;
      programInitiator = true;
      progTimer.set(PROG_DURATION);
    }
    else if (menuState == MENU_SLEEP) {
      blinkState = YAWN;
    }
  }

  if (menuIdleTimer.isExpired()) {
    blinkState = currentGame;
  }
}

void programLoop() {

  if (buttonDoubleClicked()) {//return to menu if double clicked
    blinkState = MENU;
    menuIdleTimer.set(MENU_TIMEOUT_DURATION);
    programInitiator = false;
  }
  // button cleanup
  buttonSingleClicked();

  if (progTimer.isExpired()) {
    //here we need to determine if it's safe to move to READY state
    //to qualify, all neighbors must be in PROGRAMX or READY. No learners left
    bool readyCheck = true;

    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {//only do this check on spots WITH neighbors

        byte neighbor = getLastValueReceivedOnFace(f);

        if (neighbor == PROGRAMA || neighbor == PROGRAMB || neighbor == READY) {
          // don't do anything
        }
        else {
          // if a neighbor is still
          readyCheck = false;
        }
      }
    }

    // if we are alone, we are not ready to be ready :)
    if (isAlone()) {
      readyCheck = false;
    }

    //so the face loop is over. If the readyCheck is true, we're golden
    if (readyCheck) {  // only become ready after our programming time is done and our neighbors have received the game
      blinkState = READY;
      readyTimer.set(READY_DURATION);
      if (programInitiator) { //so for the one who begins things, THIS is where they change currentGame
        currentGame = getMyKnownGame();
        //      blinkState = currentGame; // update to the game we know and shared when our neighbors have learned and are ready
      }
    }
  }

}

byte getMyKnownGame() {

  byte game;

  if (programType == PROGRAMA) {
    game = GAMEA;
  }
  else if (programType == PROGRAMB) {
    game = GAMEB;
  }

  return game;
}

void learnLoop() {
  // button cleanup
  buttonSingleClicked();
  buttonDoubleClicked();

  // we are learning here, no going to sleep now
  idleTimer.set(SLEEP_TIMEOUT_DURATION);

  if (learnTimer.isExpired()) {//this means the game has been "learned" and we can move to PROGRAM state
    if (blinkState == LEARNA) {
      currentGame = GAMEA;
      blinkState = PROGRAMA;  // now that we've learned, share the knowledge
      progTimer.set(PROG_DURATION);
    }
    else if (blinkState == LEARNB) {
      currentGame = GAMEB;
      blinkState = PROGRAMB;  // now that we've learned, share the knowledge
      progTimer.set(PROG_DURATION);
    }
  }
}

void readyLoop() {
  // button cleanup
  buttonSingleClicked();
  buttonDoubleClicked();

  if (readyTimer.isExpired()) {

    //so now we look at all neighbors and determine if all are in READY or GAME so we can transition to game state
    bool isSuroundedByCurrentGameOrReady = true;

    FOREACH_FACE(f) {

      if (!isValueReceivedOnFaceExpired(f)) {//only check occupied faces

        byte neighbor = getLastValueReceivedOnFace(f);

        if (neighbor != READY && neighbor != currentGame) {
          //this face is good
          isSuroundedByCurrentGameOrReady = false;
        }
      }
    }

    if (isSuroundedByCurrentGameOrReady) { //survived the loop while still true
      blinkState = currentGame;
      programInitiator = false;
    }
  }
}

void yawnLoop() {
  // button cleanup
  buttonSingleClicked();
  buttonDoubleClicked();

  if (sleepDelayTimer.isExpired()) {
    blinkState = KO;
  }

  if (buttonDoubleClicked()) {//return to menu if double clicked
    blinkState = MENU;
    menuIdleTimer.set(MENU_TIMEOUT_DURATION);
  }

  if (menuIdleTimer.isExpired()) {
    blinkState = SLEEP;
  }
}

/*
    DISPLAY LOOPS
*/
void displaySleep() {
  setColor(dim(BLUE, 64));
}

void displayWake() {
  setColor(YELLOW);
}

void displayProd() {
  setColor(YELLOW);
  setColorOnFace(WHITE, 1);
}

void displayGame() {
  if (blinkState == GAMEA) {
    setColor(ORANGE);
  }
  else if (blinkState == GAMEB) {
    setColor(RED);
  }
}

void displayMenu() {
  switch (menuState) {
    case MENU_PROG: setColor(OFF); setColorOnFace(GREEN, 4); break;
    case MENU_SLEEP: setColor(OFF); setColorOnFace(BLUE, 1); break;
  }
}

void displayProgram() {
  setColor(GREEN);
}

void displayLearn() {
  setColor(OFF);
  setColorOnFace(GREEN, 0);
  setColorOnFace(GREEN, 1);
  setColorOnFace(GREEN, 2);
}

void displayReady() {
  setColor(OFF);
  setColorOnFace(GREEN, 1);
  setColorOnFace(GREEN, 3);
  setColorOnFace(GREEN, 5);
}

void displayYawn() {
  setColor(BLUE);
}

void displayKO() {
  setColor(BLUE);
  setColorOnFace(WHITE, 1);
  setColorOnFace(WHITE, 3);
  setColorOnFace(WHITE, 5);
}

/*########################################
       ONLY FOR MASTER API BRANCH
  ########################################*/
bool buttonMenuPressed() {
  return buttonLongPressed();
}

