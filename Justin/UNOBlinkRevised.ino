//Color changing variables
Timer colorTimer;

byte colorChange;

Color currentColor;

byte colorValue;
byte receivedColorValue;

byte colorFreq = 25;

byte colorCount = 0;
byte receivedColorCount;

bool colorMatch;

//Face changing variables
Timer faceTimer;

byte faceChange;

byte wPixel;

byte whiteValue = 4;
byte receivedWhiteValue;

byte whiteFreq = 25;

byte whiteCount = 0;
byte receivedWhiteCount;

bool whiteMatch;

//moved variables
bool moved;

//connection variables
bool set;
byte buttonCount;

//debugging
#include "Serial.h"
ServicePortSerial Serial;

void setup() {
  Serial.begin();

  colorTimer.set(colorFreq * 100);
  faceTimer.set(whiteFreq * 100);
}

void loop() {
  //if it's alone, it has moved
  if (isAlone()) {
    moved = true;
  }

  //Color changing code
  if (colorTimer.isExpired()) {
    if (!set) {
      colorChange += 1;
      colorTimer.set(colorFreq * 100);
    }
  }

  switch (colorChange) {
    case 0: currentColor = BLUE; colorValue = 0; break;
    case 1: currentColor = RED; colorValue = 1; break;
    case 2: currentColor = YELLOW; colorValue = 2; break;
    case 3: currentColor = GREEN; colorValue = 3; break;
    case 4: colorChange = 0; break;
  }

  //Face changing code
  if (faceTimer.isExpired()) {
    if (!set) {
      faceChange += 1;
      faceTimer.set(whiteFreq * 100);
    }
  }

  switch (faceChange) {
    case 0: wPixel = 0; break;
    case 1: wPixel = 1; break;
    case 2: wPixel = 2; break;
    case 3: wPixel = 3; break;
    case 4: wPixel = 4; break;
    case 5: wPixel = 5; break;
    case 6: faceChange = 0; break;
  }

  //Color and face frequencies
  //The higher a color or face match, the faster the color/face changes
  switch (colorCount) {
    case 1: colorFreq = 23; break;
    case 2: colorFreq = 20; break;
    case 3: colorFreq = 18; break;
    case 4: colorFreq = 14; break;
    case 5: colorFreq = 10; break;
    case 6: colorFreq = 8; break;
    case 7: colorFreq = 5; break;
  }

  switch (whiteCount) {
    case 1: whiteFreq = 23; break;
    case 2: whiteFreq = 20; break;
    case 3: whiteFreq = 18; break;
    case 4: whiteFreq = 14; break;
    case 5: whiteFreq = 10; break;
    case 6: whiteFreq = 8; break;
    case 7: colorFreq = 5; break;
  }

  //Go through the faces and assign the correct colors and white pixel and send the data
  for (int i = 0; i < 6; i++) {
    if (i != wPixel) {
      setFaceColor(i, currentColor);
      //set the colorCount and colorValue to 1 value to send
      byte colorData = (colorCount * 10) + colorValue;
      //if you press the button, send the color data
      if (set) {
        setValueSentOnFace(colorData, i);
      }
    } else if (i == wPixel) {
      setFaceColor(i, WHITE);
      //set the whiteCount and whiteValue to 1 value to send
      byte faceData = (whiteCount * 10) + whiteValue;
      //if you press the button, send the white pixel data
      if (set) {
        setValueSentOnFace(faceData, i);
      }
    }
  }

  //Go through sent data and see if it was a match
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      //if not white pixel
      if (f != wPixel) {
        //get the last data sent on colored faces
        byte receivedColorData = getLastValueReceivedOnFace(f);
        //Get the colorValue sent 
        receivedColorValue = receivedColorData % 10;
        //See if colorValue sent is a match to current colorValue
        if (receivedColorValue == colorValue) {
          Serial.println(colorCount);
          colorMatch = true;
          //if the colors match, then compare Colorcounts and adjust to faster speed
          receivedColorCount = (receivedColorData - receivedColorValue) / 10;
          if (receivedColorCount > colorCount) {
            colorCount = receivedColorCount;
          }
        }
      } else if (f == wPixel) {
        byte receivedWhiteData = getLastValueReceivedOnFace(f);
        receivedWhiteValue = receivedWhiteData % 10;
        if (receivedWhiteValue == whiteValue) {
          whiteMatch = true;
          receivedWhiteCount = (receivedWhiteData - receivedWhiteValue) / 10;
          if (receivedWhiteCount > whiteCount) {
            whiteCount = receivedWhiteCount;
          }
        }
      }
    }
  }

  //Set the piece when clicked
  if (moved) {
    if (buttonSingleClicked()) {
      buttonCount += 1;
      if (buttonCount == 1) {
        set = true;
        if (colorMatch) {
          colorCount += 1;
          colorMatch = false;
        }
        if (whiteMatch) {
          whiteCount += 1;
          whiteMatch = false;
        }
      } else if (buttonCount >= 2) {
        set = false;
        buttonCount = 0;
      }
    }
  }

  //Reset the game pieces
  if (buttonMultiClicked()) {
    if (buttonClickCount() == 3) {
      set = false; 
      buttonCount = 0; 
      colorFreq = 25; 
      whiteFreq = 25; 
      colorCount = 0; 
      whiteCount = 0; 
      colorTimer.set(colorFreq * 10);
      faceTimer.set(whiteFreq * 10);
    }
  }

}
