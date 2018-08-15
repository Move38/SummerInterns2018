bool isRolling = false;
byte currentValue;
Timer rollTimer;
byte positionRandomizer = 0;

void setup() {
  // put your setup code here, to run once:
  currentValue = rand(5) + 1;
  displayValue();
}

void loop() {
  if (!isRolling) { //waiting for input to start rolling
    if (buttonSingleClicked()) {
      isRolling = true;//Begin roll!
      setValueSentOnAllFaces(0);
    }
  } else {//you're rolling. change displayed value on interval, wait for input to stop
    if (rollTimer.isExpired()) {
      currentValue = rand(5) + 1;
      displayValue();
      rollTimer.set(66);
    }

    if (buttonSingleClicked()) {
      isRolling = false;
      setValueSentOnAllFaces(currentValue);
    }
  }
}

void displayValue() {
  setColor(OFF);
  switch (currentValue) {
    case 1:
      setColorOnFace(RED, rand(5));
      break;
    case 2:
      positionRandomizer = rand(2);
      setColorOnFace(ORANGE, positionRandomizer);
      setColorOnFace(ORANGE, positionRandomizer + 3);
      break;
    case 3:
      positionRandomizer = rand(1);
      setColorOnFace(YELLOW, positionRandomizer);
      setColorOnFace(YELLOW, positionRandomizer + 2);
      setColorOnFace(YELLOW, positionRandomizer + 4);
      break;
    case 4:
      setColor(GREEN);
      positionRandomizer = rand(2);
      setColorOnFace(OFF, positionRandomizer);
      setColorOnFace(OFF, positionRandomizer + 3);
      break;
    case 5:
      setColor(BLUE);
      setColorOnFace(OFF, rand(5));
      break;
    case 6:
      setColor(MAGENTA);
      break;
  }
}

