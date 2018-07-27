bool isHappy;
byte numNeighbors;

bool happyBlinkOn = true;
byte tileState; //a 2-digit made of up team first and fractureState second
byte team; //0-3, represents colors
int playerHues[4] = {20, 46, 91, 124};//these are the hues of the color, since we use HSB in here for animation
byte fractureState; //0-2, represents the state of the fracture

enum fractureStates {NOMINAL, FRACTURED, RESOLVING};
enum faceStates {EMPTY, NEIGHBORED, MISSING, NEWFRIEND};

byte faceInfo[6]; //describes the state of each side, neighbored or not and distressed or not. Affects animations

Timer resolveTimer;
Timer happyTimer;
Timer distressTimer;

void setup() {
  team = 0;
  fractureState = NOMINAL;
  setColor(makeColorHSB(playerHues[team], 255, 255));

  //we need to fill the faceInfo array initially before any loops begin
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      faceInfo[f] = NEIGHBORED;
    } else {
      faceInfo[f] = EMPTY;
    }
  }
}

void loop() {

  //first, we need to listen for double clicks to change team
  if (buttonDoubleClicked()) {
    team++;
    if (team > 3) {
      team = 0;
    }
    setColor(makeColorHSB(playerHues[team], 255, 255)); //set the blink to the team color
  }

  //determine which loop to run
  switch (fractureState) {
    case NOMINAL:
      nominalLoop();
      break;
    case FRACTURED:
      fractureLoop();
      break;
    case RESOLVING:
      resolveLoop();
      break;
  }

  //Now determine if this tile is happy. Default to true, then disqualify if we find an alike neighbor or fewer than 2 neighbors
  isHappy = true;
  numNeighbors = 0;
  FOREACH_FACE(f) {
    if (faceInfo[f] == NEIGHBORED || faceInfo[f] == NEWFRIEND) { //this means there is something there
      numNeighbors++;
      if (getLastValueReceivedOnFace(f) / 10 == team) { //this means this neighbor is the same color as us. Oh no!
        isHappy = false;
      }
    }
  }
  if (numNeighbors < 2) { //if numNeighbors never reached 2, it's not happy
    isHappy = false;
  }

  //now that everything is resolved, remake the tileState and make sure it is sending
  tileState = team * 10;
  tileState += fractureState;
  setValueSentOnAllFaces(tileState);
}

void nominalLoop() {
  //this is the behavior when all the blinks are together

  //begin by evaluating all faces and looking for suddenly missing neighbors
  FOREACH_FACE(f) {
    if (faceInfo[f] == NEIGHBORED && isValueReceivedOnFaceExpired(f)) {
      //oh no, something has gone
      faceInfo[f] = MISSING;//distressed empty
      fractureState = FRACTURED;//begin fractured state
    }

    if (getLastValueReceivedOnFace(f) % 10 == FRACTURED) {//this is a lookout for neighbors in fractured state,
      fractureState = FRACTURED;//this means a neighbor has become fractured, you should also be on alert
    }
  }

  if (happyTimer.isExpired() && isHappy) {//this is the happiness animation
    if (happyBlinkOn) { //here we turn the faces off for a short period
      happyTimer.set(200);
      setColor(OFF);
      happyBlinkOn = false;
    } else if (!happyBlinkOn) { //here we turn faces back on for a longer period
      happyTimer.set(800);
      setColor(makeColorHSB(playerHues[team], 255, 255));
      happyBlinkOn = true;
    }
  }

  if (!isHappy) { //this is here to make sure the blink doesn't become unhappy while the colors are off
    setColor(makeColorHSB(playerHues[team], 255, 255));
  }

  if (fractureState == FRACTURED) {//just making sure the blink is on and colored when it transitions to fracture state
    setColor(makeColorHSB(playerHues[team], 255, 255));
    happyBlinkOn = true;
  }

  //after the face loop is over, we do some things if it found a fracture
  //this part is not needed now that the animation thing is in place
  //  if (fractureState == FRACTURED) {
  //    setColor(RED);
  //    set the newly missing faces to white
  //    FOREACH_FACE(f) {
  //      if (faceInfo[f] == MISSING) {
  //        setFaceColor(f, WHITE);
  //      }
  //    }
  //  }
}


void fractureLoop() {
  FOREACH_FACE(f) {
    //let keep looking for missing neighbors here
    if (faceInfo[f] == NEIGHBORED && isValueReceivedOnFaceExpired(f)) {
      //oh no, something has gone
      faceInfo[f] = MISSING;
      setFaceColor(f, WHITE);
    }

    //here we see if the distress timer has expired and do the animation stuff
    if (distressTimer.isExpired()) {
      distressTimer.set(200);//reset the timer
      FOREACH_FACE(f) {//randomly fade the colors to white
        if (faceInfo[f] ==  MISSING) {
          //setFaceColor(f, makeColorHSB(playerHues[team], rand(255), 255));//uses player color, a random saturation, and full brightness
          setFaceColor(f, makeColorHSB(0, rand(100) + 155, 255)); //lava version
        }
      }
    }

    //now we'll look for new neighbors on EMPTY or MISSING faces, which means the fracture has come to an end
    if (!isValueReceivedOnFaceExpired(f) && (faceInfo[f] == EMPTY || faceInfo[f] == MISSING)) {
      //aha, there is a new neighbor
      faceInfo[f] = NEWFRIEND;//new neighbor state, resolved later
      fractureState = RESOLVING;
    }

    // last, we just look out for neighbors who are resolving, which means we should be too
    if (!isValueReceivedOnFaceExpired(f) && getLastValueReceivedOnFace(f) % 10 == RESOLVING) {
      fractureState = RESOLVING;//this means a neighbor has started to resolve, regardless of
    }
  }

  //like before, if we made it through there and the state has changed, do some things
  if (fractureState == RESOLVING) {
    //set the newly paired faces to white to show resolution
    //setColor(GREEN);
    FOREACH_FACE(f) {
      if (faceInfo[f] == NEWFRIEND) {
        setFaceColor(f, WHITE);
      }
    }

    resolveTimer.set(500);
  }
}

void resolveLoop() {
  //this is the behavior when the fracture has been resolved. It only occurs for the duration of one short timer
  //while this is running, continue checking for new neighbors
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f) && (faceInfo[f] == EMPTY || faceInfo[f] == MISSING)) {
      //aha, there is a new neighbor
      faceInfo[f] = NEWFRIEND;//new neighbor state, resolved later
      setFaceColor(f, WHITE);
    }
  }

  //when this state comes to an end, we reset all distressed face states to the nominal state
  if (resolveTimer.isExpired()) {
    FOREACH_FACE(f) {
      if (!isValueReceivedOnFaceExpired(f)) {
        faceInfo[f] = NEIGHBORED;//occupied, nominal
      } else {
        faceInfo[f] = EMPTY;//empty, nominal
      }
    }

    setColor(makeColorHSB(playerHues[team], 255, 255));
    fractureState = NOMINAL;
  }
}

