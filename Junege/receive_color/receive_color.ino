Color colors[] = { RED, ORANGE, YELLOW, GREEN, BLUE };
byte randColor = rand(4);
byte currentColor;

void setup() {
  setColor(colors[randColor]); //on wake, set to a random color (will always be the same due to seed)

}

void loop() {


  if (buttonDown()) {
    randColor = rand(4);
  }

  if (!buttonDown()) {
    setColor(colors[randColor]);
    currentColor = randColor;
  }

  switch (currentColor) {

    case 0: setValueSentOnAllFaces(0); break;
    case 1: setValueSentOnAllFaces(1); break;
    case 2: setValueSentOnAllFaces(2); break;
    case 3: setValueSentOnAllFaces(3); break;
    case 4: setValueSentOnAllFaces(4); break;
  }

  if (!isAlone()) {

    receiveColor();

  }

}

void receiveColor() {

  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      byte colorReceived = getLastValueReceivedOnFace(f);

      switch (colorReceived) {
        case 0: setFaceColor(f, RED); break;
        case 1: setFaceColor(f, ORANGE); break;
        case 2: setFaceColor(f, YELLOW); break;
        case 3: setFaceColor(f, GREEN); break;
        case 4: setFaceColor(f, BLUE); break;

      }


    }
  }


}







