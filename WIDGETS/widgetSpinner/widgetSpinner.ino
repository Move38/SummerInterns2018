Color spinColors[] = {RED, ORANGE, YELLOW, GREEN, BLUE, MAGENTA};
byte currentFace = 0;
bool isSpinning = false;
bool isSlowing = false;
Timer spinTimer;
float spinIncrement = 20;
byte spinRandomizer;

void setup() {
  // put your setup code here, to run once:
  setColorWheel();
  setColorOnFace(WHITE, currentFace);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!isSpinning && buttonSingleClicked()) {
    //begin spin!
    isSpinning = true;
    spinRandomizer = rand(5) + 54;
    spinIncrement = 20;
    setValueSentOnAllFaces(0);
  }

  if (isSpinning && !isSlowing) {
    spinningLoop();
  } else if (isSpinning && isSlowing) {
    slowingLoop();
  }
}

void spinningLoop() {
  if (spinTimer.isExpired()) {
    spinRandomizer--;
    setColorWheel();
    currentFace = nextClockwise(currentFace);
    setColorOnFace(WHITE, currentFace);
    spinTimer.set(spinIncrement);
  }

  if (spinRandomizer == 0) {
    isSlowing = true;
  }
}

void slowingLoop() {
  if (spinTimer.isExpired()) {
    spinIncrement = spinIncrement * 1.05;
    setColorWheel();
    currentFace = nextClockwise(currentFace);
    setColorOnFace(WHITE, currentFace);
    spinTimer.set(spinIncrement);
  }

  if (spinIncrement > 350) {
    isSpinning = false;
    isSlowing = false;
    setColorOnFace(spinColors[currentFace], currentFace);
    setValueSentOnAllFaces(currentFace+1);
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

void setColorWheel() {
  FOREACH_FACE(f) {
    setColorOnFace(dim(spinColors[f], 16), f);
  }
}

