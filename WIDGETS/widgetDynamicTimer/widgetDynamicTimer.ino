enum timerStates {WAITING, RECORDING, TIMING};
byte timerState = WAITING;
int timerLength = 30;
int countdown;
byte currentFace = 0;

Timer secondTick;
Timer offsetTimer;

void setup() {
  // put your setup code here, to run once:
  setColor(RED);
  setColorOnFace(WHITE, currentFace);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (timerState) {
    case WAITING:
      waitingLoop();
      break;
    case RECORDING:
      recordingLoop();
      break;
    case TIMING:
      timingLoop();
      break;
  }

}

void waitingLoop() {
  if (buttonSingleClicked()) {
    timerState = TIMING;
    secondTick.set(1000);
    offsetTimer.set(500);
    countdown = timerLength;
    setColor(OFF);
    setColorOnFace(WHITE, currentFace);
  }

  if (buttonDoubleClicked()) {
    timerState = RECORDING;
    secondTick.set(1000);
    offsetTimer.set(500);
    timerLength = 0;
    setColor(BLUE);
    setColorOnFace(WHITE, currentFace);
  }
}

void recordingLoop() {
  if (secondTick.isExpired()) {
    timerLength++;//increment timer
    currentFace = nextCounterclockwise(currentFace);
    setColorOnFace(WHITE, currentFace);
    secondTick.set(1000);
  }

  if (offsetTimer.isExpired()) { //this deactivates the active pixel halway through the second
    setColor(BLUE);
    offsetTimer.set(1000);
  }

  if (buttonSingleClicked()) { //end recording, set back to waiting mode
    setColor(RED);
    setColorOnFace(WHITE, currentFace);
    timerState = WAITING;
  }
}

void timingLoop() {
  if (secondTick.isExpired()) {
    countdown--;//decrement the countdown

    currentFace = nextClockwise(currentFace);
    if (countdown <= 6) { //turn red in last 6 seconds
      setColorOnFace(RED, currentFace);
    } else {
      setColorOnFace(WHITE, currentFace);
    }

    if (countdown == 0) { //it's over!
      setColor(RED);
      setColorOnFace(WHITE, currentFace);
      timerState = WAITING;
    }

    secondTick.set(1000);
  }

  if (offsetTimer.isExpired()) { //this deactivates the active pixel halway through the second
    setColor(OFF);
    offsetTimer.set(1000);
  }
}

byte nextClockwise (byte face) {
  if (face == 5) {
    return 0;
  } else {
    return face + 1;
  }
}

byte nextCounterclockwise (byte face) {
  if (face == 0) {
    return 5;
  } else {
    return face - 1;
  }
}
