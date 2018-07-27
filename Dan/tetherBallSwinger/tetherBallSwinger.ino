//first, a set of values that help determine the player behavior and change with player number
byte playerNumber = 1;
Color playerColor;
byte backswingFace;
byte hitFace;
byte followthroughFace;

bool swingReady = true;

Timer animTimer;
bool isAnimating = false;
byte animFrame = 0;

void setup() {
  beginSwing(1);
}

void loop() {
  if (isAlone()) { //the swing is primed when you are alone. You can't just connect and swing forever
    swingReady = true;
  } else {//not alone
    if (swingReady) {//is not alone, and also swing is ready
      FOREACH_FACE(f) {
        if (!isValueReceivedOnFaceExpired(f)) { //you've found the pole. SWING!
          swingReady = false;//this prevents an infinite swing
          beginSwing(f);
        }
      }
    }
  }

  //on double click, we switch player
  if (buttonDoubleClicked()) {
    if (playerNumber == 1) {
      playerNumber = 2;
    } else if (playerNumber == 2) {
      playerNumber = 1;
    }
    beginSwing(1);
  }

  //if we are animating, we need to be listening for the animTimer and progressing the animation
  if (isAnimating) {
    //it's only in here that we check for the expiring timer
    if (animTimer.isExpired()) {
      if (animFrame == 1) { //right now backswing and face are lit. Move to next set
        animTimer.set(100);
        setFaceColor(backswingFace, OFF);
        setFaceColor(followthroughFace, playerColor);
        animFrame = 2;
      } else if (animFrame == 2) { //right now face and followthrough are lit. End animation
        setColor(OFF);
        isAnimating = false;
        animFrame = 0;
        setValueSentOnAllFaces(0);
      }
    }
  }
}

void beginSwing(byte face) {
  //first we determine position and color of swing via player and face
  hitFace = face;
  if (playerNumber == 1) { //set backswing for clockwise hit player
    playerColor = BLUE;
    backswingFace = nextClockwise(hitFace);
    followthroughFace = nextCounterclockwise(hitFace);
  } else if (playerNumber == 2) {
    playerColor = RED;
    backswingFace = nextCounterclockwise(hitFace);
    followthroughFace = nextClockwise(hitFace);
  }

  //now we begin the animation and set the timer
  animTimer.set(100);
  animFrame = 1;
  isAnimating = true;
  setFaceColor(backswingFace, playerColor);
  setFaceColor(hitFace, playerColor);
  setValueSentOnFace(playerNumber, hitFace);
}

byte nextCounterclockwise (byte face) {
  if (face == 0) {
    return 5;
  } else {
    return face - 1;
  }
}

byte nextClockwise (byte face) {
  if (face == 5) {
    return 0;
  } else {
    return face + 1;
  }
}

