ran#include "Serial.h"
ServicePortSerial Serial;

byte currentFace;
bool gameBegan;
bool gameOver;
bool travelingClockwise;

bool hitEligible;

Timer ballTimer;
int baseIncrement = 250;
int wrap = 0;
int wrapIncrement = 3;//increase in speed for each frame of animation
int hitBonus = 0;
int hitIncrement = 10;//increase in speed for each constructive hit

void setup() {
  Serial.begin();
  // put your setup code here, to run once:
  currentFace = rand(5);
  hitEligible = true;
  setFaceColor(currentFace, WHITE);
  wrap = 0;
  hitBonus = 0;
  gameBegan = false;
  gameOver = false;
}

void loop() {
  //first thing, you can reset the pole by double clicking
  if (buttonDoubleClicked()) {
    setColor(OFF);
    setup();
  }

  //determine which loop to be using
  if (!gameBegan && !gameOver) {
    startingLoop();
  } else if (gameBegan && !gameOver) {
    gameLoop();
  } else if (gameBegan && gameOver) {
    victoryLoop();
  }
}

void startingLoop() { //only called before the first swing is registered
  if (!isValueReceivedOnFaceExpired(currentFace)) {
    //once we see something on that face, figure out which player it is
    if (getLastValueReceivedOnFace(currentFace) == 1) { //you've been hit by player 1, go clockwise
      setFaceColor(currentFace, BLUE); //set the face to the player color for one frame
      ballTimer.set(baseIncrement);
      hitEligible = false;
      travelingClockwise = true;
      gameBegan = true;
    } else if (getLastValueReceivedOnFace(currentFace) == 2) { //you've been hit by player 2, go counterclockwise
      setFaceColor(currentFace, RED); //set the face to the player color for one frame
      travelingClockwise = false;
      ballTimer.set(baseIncrement);
      hitEligible = false;
      gameBegan = true;
    }
  }
}

void gameLoop() { //basically the loop of the game, but only starts after the first swing
  //here we listen on the currentFace for input from players
  if (!isValueReceivedOnFaceExpired(currentFace) && hitEligible) {
    hitEligible = false; //restored to true on the next animation frame
    if (getLastValueReceivedOnFace(currentFace) == 1) { //you've been hit by player 1, go clockwise
      setFaceColor(currentFace, BLUE); //set the face to the player color for one frame
      if (travelingClockwise) { //this means you were already going this way. Speed up!
        hitBonus += hitIncrement;
      } else { //you weren't going this way. Slow down before you reverse!
        hitBonus = 0;
      }
      travelingClockwise = true;
    } else if (getLastValueReceivedOnFace(currentFace) == 2) { //you've been hit by player 2, go counterclockwise
      setFaceColor(currentFace, RED); //set the face to the player color for one frame
      if (!travelingClockwise) { //this means you were already going this way. Speed up!
        hitBonus  += hitIncrement;
      } else { //you weren't going this way. Slow down before you reverse!
        hitBonus = 0;
      }
      travelingClockwise = false;
    }
  }

  if (ballTimer.isExpired()) { //the main animation function. Moves the ball one space in one direction. Also polices the hitEligible bool
    setColor(OFF);
    if (travelingClockwise) {
      setFaceColor(currentFace, dim(BLUE, 51));
      currentFace = nextClockwise(currentFace);
      wrap += wrapIncrement;
    } else {
      setFaceColor(currentFace, dim(RED, 51));
      currentFace = nextCounterclockwise(currentFace);
      wrap -= wrapIncrement;
    }

    //victory check
    if (abs(wrap) >= wrapIncrement * 60) { //ten revolutions from the starting point
      gameOver = true;

      //determine winning color
      if (travelingClockwise) {
        setColor(BLUE);
      } else {
        setColor(RED);
      }

    } else {//carry on with the animation
      setFaceColor(currentFace, WHITE);
      ballTimer.set(baseIncrement - abs(wrap) - hitBonus);
      Serial.println(baseIncrement - abs(wrap) - hitBonus);
      hitEligible = true;//now that an animation has happened, we are eligible for another hit. Avoids speed problems
    }
  }
}

void victoryLoop() {
  //nothing happens here now, only that other loop logic doesn't happen
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
