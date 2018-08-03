byte flips;
Color faceColor = YELLOW;

bool isFlipping;
Timer flipIncrements;
byte currentFrame;

void setup() {
  // put your setup code here, to run once:
  setColor(faceColor);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (buttonSingleClicked() && !isFlipping) {
    //this means you are available for flipping, and the button has been pressed. Begin flip!
    isFlipping = true;
    flips = rand(1) + 15;
    currentFrame = 1;
  }

  if (isFlipping && flipIncrements.isExpired()) { //animate the next frame of the animation
    flipIncrements.set(50);
    switch (currentFrame) {//you are currently in tilted  mode. transition to full face
      case 1:
        setColor(faceColor);
        currentFrame = 2;
        break;
      case 2://you are currently in full face. Change to tilted
        setColor(OFF);
        setColorOnFace(faceColor, 0);
        setColorOnFace(faceColor, 3);
        currentFrame = 3;
        break;
      case 3://you are currently in tilted. Change color
        setColor(OFF);
        currentFrame = 4;
        break;
      case 4://you are currently in off. Change color and turn on tilted
        if (faceColor == YELLOW) {//switch colors
          faceColor = MAGENTA;
        } else {
          faceColor = YELLOW;
        }
        setColorOnFace(faceColor, 0);
        setColorOnFace(faceColor, 3);
        currentFrame = 1;
        break;
    }

    if (currentFrame == 2) {//this means you are presenting a coin face
      flips--;
    }

    if (flips == 0) {//this is the face you are landing on. Stop flipping
      isFlipping = false;
    }
  }
}
