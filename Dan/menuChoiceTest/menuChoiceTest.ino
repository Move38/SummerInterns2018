enum menuStates {GAME, MENU, PROGRAM, SLEEP};
byte menuState;
bool programChosen = true;

Timer timeoutTimer;

void setup() {
  // put your setup code here, to run once:
  setColor(WHITE);
  menuState = GAME;
}

void loop() {
  if (buttonMenuPressed()) {
    timeoutTimer.set(10000);
    menuState = MENU;
    setColorOnFace(RED, 0);
    setColorOnFace(RED, 1);
    setColorOnFace(RED, 2);
    setColorOnFace(dim(BLUE, 50), 3);
    setColorOnFace(dim(BLUE, 50), 4);
    setColorOnFace(dim(BLUE, 50), 5);
  }

  if (menuState == MENU) {
    menuLoop();
  }
}

void menuLoop() {
  //first, make sure you haven't timed out
  if (timeoutTimer.isExpired()) {
    menuState = GAME;
    setColor(WHITE);
  }

  if (buttonSingleClicked()) {
    timeoutTimer.set(10000);
    //switch menu choice
    if (programChosen) {//set to sleep choice
      programChosen = false;
      setColorOnFace(dim(RED, 50), 0);
      setColorOnFace(dim(RED, 50), 1);
      setColorOnFace(dim(RED, 50), 2);
      setColorOnFace(BLUE, 3);
      setColorOnFace(BLUE, 4);
      setColorOnFace(BLUE, 5);
    } else {//set to program choice
      programChosen = true;
      setColorOnFace(RED, 0);
      setColorOnFace(RED, 1);
      setColorOnFace(RED, 2);
      setColorOnFace(dim(BLUE, 50), 3);
      setColorOnFace(dim(BLUE, 50), 4);
      setColorOnFace(dim(BLUE, 50), 5);
    }
  }

  if (buttonDoubleClicked()){
    timeoutTimer.set(10000);
    if(programChosen){
      setColor(RED);
    } else {
      setColor(BLUE);
    }
  }
}
