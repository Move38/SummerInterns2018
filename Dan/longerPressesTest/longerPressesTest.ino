enum menuStates {GAME, PROGRAM, SLEEP};
byte menuState;

Timer programBlinkTimer;
Timer sleepFadeTimer;

Timer timeoutTimer;

void setup() {
  // put your setup code here, to run once:
  setColor(WHITE);
  menuState = GAME;
}

void loop() {
  if (buttonMenuPressed()) {
    setColor(RED);
    timeoutTimer.set(10000);
    menuState = PROGRAM;
  }

  if (buttonSleepPressed()) {
    setColor(BLUE);
    timeoutTimer.set(10000);
    menuState = SLEEP;
  }

  if(menuState == PROGRAM){
   programLoop(); 
  }

  if(menuState == SLEEP){
   programLoop(); 
  }
}

void programLoop() {
  //for now, this just holds the animation info for the program state
  //first, make sure you haven't timed out
  if (timeoutTimer.isExpired()){
    menuState = GAME;
    setColor(WHITE);
  }
}

void sleepLoop() {
  //for now, this just holds the animation info for the sleep state
    //first, make sure you haven't timed out
  if (timeoutTimer.isExpired()){
    menuState = GAME;
    setColor(WHITE);
  }
}

