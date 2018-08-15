enum blinkStates {SLEEP, WAKE, PROD, GAMEA, GAMEB, MENU, PROGRAMA, PROGRAMB, LEARNA, LEARNB, READY, YAWN, KO};
byte blinkState;
bool menuToggle = true;
byte currentGame = GAMEB;
byte programType = PROGRAMA;
bool programInitiator = false;
Timer animTimer;
int animFrame = 0;

Timer idleTimer;
Timer menuIdleTimer;
Timer learnTimer;

void setup() {
  // put your setup code here, to run once:
  blinkState = WAKE;
  idleTimer.set(60000);
}

void loop() {
  // first, the loop switch
  switch (blinkState) {
    case SLEEP:
      sleepLoop();
      break;
    case WAKE:
    case PROD:
      wakeLoop();
      break;
    case GAMEA:
    case GAMEB:
      gameLoop();
      break;
    case MENU:
      menuLoop();
      break;
    case PROGRAMA:
    case PROGRAMB:
      programLoop();
      break;
    case LEARNA:
    case LEARNB:
      learnLoop();
      break;
    case READY:
      readyLoop();
      break;
    case YAWN:
    case KO:
      yawnLoop();
      break;
  }

  //now some housekeeping, like making sure the sleep timer is reset after ANY interaction, and that you go to sleep if it expires
  if (buttonDown()) {
    idleTimer.set(120000);
  }

  if (idleTimer.isExpired() && blinkState != SLEEP) {
    setColor(OFF);
    blinkState = SLEEP;
    buttonDoubleClicked();
    buttonMenuPressed();
  }

  setValueSentOnAllFaces(blinkState);
}

void sleepLoop() {
  if (buttonSingleClicked()) { //someone has hit the button. This is method 1 of waking up
    blinkState = WAKE;
  }

  FOREACH_FACE(f) { //this looks for neighbors in WAKE state. Method 2 of waking up
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) == PROD) {
      blinkState = WAKE;
      idleTimer.set(60000);
    }
  }

  if (blinkState == WAKE) {
    animFrame = 0;
  }
}

void wakeLoop() {
  if (animTimer.isExpired()) { //do the wake animation.
    animFrame++;
    setColor(dim(YELLOW, animFrame * 17));
    animTimer.set(100);
  }

  if (animFrame > 3) { //you are halfway through wakeup. Start waking neighbors
    blinkState = PROD;
  }

  if (animFrame == 15) { //last frame of animation
    blinkState = currentGame;
    if (blinkState == GAMEA) {
      setColor(ORANGE);
    } else if (blinkState == GAMEB) {
      setColor(RED);
    }
  }
}

/*
   Play game until one of the following is sensed
   1. Menu Action (alone and pressed for > n seconds)
   2. Neighbor is putting me to sleep (KO_STATE)
   3. Neighbor is programming me (PROGRAM_STATE)
*/
void gameLoop() {
  if (animTimer.isExpired()) {// do a basic game animation thing
    if (blinkState == GAMEA) {//just a simple animation that turns on and off faces randomly
      setColorOnFace(ORANGE, rand(5));
      setColorOnFace(OFF, rand(5));
      setColorOnFace(ORANGE, rand(5));
      animTimer.set(rand(1) * 100 + 100);
    } else if (blinkState == GAMEB) {//a simple animation that randomly fades a face
      setColorOnFace(dim(RED, rand(255)), rand(5));
      animTimer.set(150);
    }
  }

  //now we need to start looking out for menu press, but only when alone
  if (buttonMenuPressed()) {//you have to do this first to set the flag to false on every check
    if (isAlone()) {
      blinkState = MENU;
      menuIdleTimer.set(10000);
      setColorOnFace(dim(GREEN, 16), 0);
      setColorOnFace(dim(GREEN, 16), 1);
      setColorOnFace(dim(GREEN, 16), 2);
      setColorOnFace(dim(BLUE, 16), 3);
      setColorOnFace(dim(BLUE, 16), 4);
      setColorOnFace(dim(BLUE, 16), 5);
      animFrame = 0;
    }
  }

  //we need to look out for a KO or programming signals from neighbors
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//you found a neighbor
      if (getLastValueReceivedOnFace(f) == KO) { //KO command
        blinkState = YAWN;
        menuIdleTimer.set(3000);//secondhand sleep timer
        animFrame = 0;
      } else if (getLastValueReceivedOnFace(f) == PROGRAMA) {
        blinkState = LEARNA;
        learnTimer.set(6000);
        setColor(GREEN);
        animFrame = 0;
      } else if (getLastValueReceivedOnFace(f) == PROGRAMB) {
        blinkState = LEARNB;
        learnTimer.set(6000);
        setColor(GREEN);
        animFrame = 0;
      }
    }//end of found neighbor statement
  }//end of foreachface loop
}

void menuLoop() {
  if (buttonSingleClicked()) {
    menuToggle = !menuToggle;//reverse the toggle
    setColorOnFace(dim(GREEN, 16), 0);//reset all the dimness, let the next thing correct for choice
    setColorOnFace(dim(GREEN, 16), 1);
    setColorOnFace(dim(GREEN, 16), 2);
    setColorOnFace(dim(BLUE, 16), 3);
    setColorOnFace(dim(BLUE, 16), 4);
    setColorOnFace(dim(BLUE, 16), 5);
    menuIdleTimer.set(10000);
  }

  if (animTimer.isExpired()) {
    animFrame++;
    if (menuToggle) { //menu toggle true is in program choice
      setColorOnFace(GREEN, 0);//reset all the greens
      setColorOnFace(GREEN, 1);
      setColorOnFace(GREEN, 2);
      setColorOnFace(WHITE, animFrame % 3);//set one white
      animTimer.set(300);
    } else if (!menuToggle) { //menu toggle false is in sleep choice
      setColorOnFace(dim(BLUE, abs(animFrame - 15) * 17), 3);
      setColorOnFace(dim(BLUE, abs(animFrame - 15) * 17), 4);
      setColorOnFace(dim(BLUE, abs(animFrame - 15) * 17), 5);
      animTimer.set(100);
    }

    if (animFrame == 30) {
      animFrame = 0;
    }
  }

  if (buttonDoubleClicked()) { //type selected, move to appropriate mode
    if (menuToggle) {//head to the correct programming mode
      blinkState = programType;
      programInitiator = true;
    } else if (!menuToggle) {
      blinkState = YAWN;
      menuIdleTimer.set(10000);
      animFrame = 0;
    }
  }

  if (menuIdleTimer.isExpired()) {
    blinkState = currentGame;
    if (blinkState == GAMEA) {
      setColor(ORANGE);
    } else if (blinkState == GAMEB) {
      setColor(RED);
    }
  }
}

void programLoop() {
  if (animTimer.isExpired()) {//just the animation
    animFrame++;
    setColor(GREEN);
    setColorOnFace(WHITE, animFrame % 3);
    setColorOnFace(WHITE, (animFrame % 3) + 3);
    animTimer.set(100);
  }

  if (buttonDoubleClicked()) {//return to menu if double clicked
    blinkState = MENU;
    setColorOnFace(dim(GREEN, 16), 0);//reset all the dimness, let the next thing correct for choice
    setColorOnFace(dim(GREEN, 16), 1);
    setColorOnFace(dim(GREEN, 16), 2);
    setColorOnFace(dim(BLUE, 16), 3);
    setColorOnFace(dim(BLUE, 16), 4);
    setColorOnFace(dim(BLUE, 16), 5);
    animFrame = 0;
    menuIdleTimer.set(10000);
    programInitiator = false;
  }

  //here we need to determine if it's safe to move to READY state
  //to qualify, all neighbors must be in PROGRAMX or READY. No learners left
  bool readyCheck = true;

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//only do this check on spots WITH neighbors

      byte neighbor = getLastValueReceivedOnFace(f);

      if (neighbor != PROGRAMA && neighbor != PROGRAMB && neighbor != READY) {
        //in some other state
        readyCheck = false;
      }
    }
  }

  // if we are alone, we are not ready to be ready :)
  if (isAlone()) {
    readyCheck = false;
  }

  //so the face loop is over. If the readyCheck is true, we're golden
  if (readyCheck) {
    blinkState = READY;
    setColor(dim(GREEN, 16));//just set it to a dim green
    if (programInitiator) { //so for the one who begins things, THIS is where they change currentGame
      currentGame = getMyKnownGame();
    }
  }


  if (menuIdleTimer.isExpired()) {
    blinkState = currentGame;
    if (blinkState == GAMEA) {
      setColor(ORANGE);
    } else if (blinkState == GAMEB) {
      setColor(RED);
    }
    programInitiator = false;
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
  if (animTimer.isExpired()) {//a simple green fading animation
    animFrame ++;
    animTimer.set(100);
    setColor(dim(GREEN, abs(animFrame - 15) * 17));
  }

  if (animFrame == 30) {
    animFrame = 0;
  }

  if (learnTimer.isExpired()) {//this means the game has been "learned" and we can move to PROGRAM state
    if (blinkState == LEARNA) {
      currentGame = GAMEA;
      blinkState = PROGRAMA;
    } else if (blinkState == LEARNB) {
      currentGame = GAMEB;
      blinkState = PROGRAMB;
    }
  }
}

void readyLoop() {
  //so now we look at all neighbors and determine if all are in READY or GAME so we can transition to game state
  bool gameCheck = false;
  
  FOREACH_FACE(f) {
  
    if (!isValueReceivedOnFaceExpired(f)) {//only check occupied faces
      
      byte neighbor = getLastValueReceivedOnFace(f);
      
      if (neighbor == READY || neighbor == currentGame) {
        //this face is good
        gameCheck = true;
      }
    }
  }

  if (gameCheck) { //survived the loop while still true
    blinkState = currentGame;
    if (currentGame == GAMEA) {
      setColor(ORANGE);
    } else if (currentGame = GAMEB) {
      setColor(RED);
    }
    animFrame = 0;
    // TODO: Why is the idle timer getting set here? Isn't that for putting a Blink to fake sleep?
    idleTimer.set(1000);
  }
}

void yawnLoop() {
  if (animTimer.isExpired()) {
    animFrame++;
    setColor(dim(BLUE, abs(animFrame - 15) * 17));
    animTimer.set(100);
  }

  if (animFrame == 3) {
    blinkState = KO;
  }

  if (animFrame == 30) {
    animFrame = 0;
  }

  if (buttonDoubleClicked()) {//return to menu if double clicked
    blinkState = MENU;
    setColorOnFace(dim(GREEN, 16), 0);//reset all the dimness, let the next thing correct for choice
    setColorOnFace(dim(GREEN, 16), 1);
    setColorOnFace(dim(GREEN, 16), 2);
    setColorOnFace(dim(BLUE, 16), 3);
    setColorOnFace(dim(BLUE, 16), 4);
    setColorOnFace(dim(BLUE, 16), 5);
    animFrame = 0;
    menuIdleTimer.set(10000);
  }

  if (menuIdleTimer.isExpired()) {
    blinkState = SLEEP;
    setColor(OFF);
    buttonDoubleClicked();
    buttonMenuPressed();
  }
}

/*########################################
 *     ONLY FOR MASTER API BRANCH
 *########################################*/
bool buttonMenuPressed() {
  return buttonLongPressed();
}

