/*
  This code will make every Blink switch between two colors. When a Blink is triple clicked, it will receive the 2 colors a neighboring Blink is using and switch between those colors.
*/

//Timer that controls when the blinks will switch colors
Timer blinkTimer;

//When this becomes true/false, it changes which color value it needs
bool switchColor;

//These are the values that will be associated with specific colors
byte colorVal1;
byte colorVal2;

//Depending on the colorVals, these colors will change in loop()
Color color1;
Color color2;

void setup() {
  //Set the blinkTimer and set the first and second colors to RED and Blue so we know it's switching colors at first
  blinkTimer.set(500);
  colorVal1 = 0;
  colorVal2 = 1;
}

void loop() {

  //If colorVal1 = x then the first color is *color*
  switch (colorVal1) {
    case 0: color1 = RED; break;
    case 1: color1 = BLUE; break;
    case 2: color1 = GREEN; break;
  }

  //If colorVal2 = x then the second color is *color*
  switch (colorVal2) {
    case 0: color2 = RED; break;
    case 1: color2 = BLUE; break;
    case 2: color2 = GREEN; break;
  }

  //When the button is pressed once, add 1 to colorVal1 and change the first color
  if (buttonSingleClicked()) {
    colorVal1 += 1;
  }

  //When colorVal1 becomes greater than 2, go back to 0 so it loops between the colors
  if (colorVal1 > 2) {
    colorVal1 = 0;
  }

  //When the button is double clicked, add 1 to colorval2 and change the second color
  if (buttonDoubleClicked()) {
    colorVal2 += 1;
  }

  if (colorVal2 > 2) {
    colorVal2 = 0;
  }

  //When the blinkTimer expires, see if switch color is true/false and make it the opposite to alternate between the first and second color.
  //Then set the blinkTimer to 500ms again.
  if (blinkTimer.isExpired()) {
    if (!switchColor) {
      switchColor = true;
    } else if (switchColor) {
      switchColor = false;
    }
    blinkTimer.set(500);
  }

  //When the switchColor is not true, show the 1st color. If switchColor is false, show the second color.
  if (!switchColor) {
    setColor(color1);
  } else if (switchColor) {
    setColor(color2);
  }

  /*
     This is where it gets a bit tricky

     We will send both the first and second colors to other neighboring Blinks by making the two values just one.


     byte data = (colorVal1 * 10) + colorVal2;


     So in this code we will be making colorVal1 the tens digit and the colorVal2 the singles digit

     EXAMPLE: Color1 = BLUE & Color2 = GREEN

     colorVal1 = 1 for BLUE & colorVal2 = 2 for GREEN

     (1 * 10) + 2 = 12 <-- This is the single value we will send to other Blinks

     When another Blink receives this data, it needs to dissect this data back to the original values

     colorVal2 = 12 % 10 = 2

     colorVal1 = (12 - (12 % 10 = 2)) / 10 = 1

  */


  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {
      //get the single color value from a neighboring Blink
      byte receivedData = getLastValueReceivedOnFace(f);
      if (buttonMultiClicked()) {
        //When the Blink is triple clicked, dissect the single value and assign those values to colorVal1 & colorVal2
        if (buttonClickCount() == 3) {
          colorVal2 = receivedData % 10;
          colorVal1 = (receivedData - (receivedData % 10)) / 10;
        }
      }
    }
  }

  //Take your colorVals and make them a single value
  byte data = (colorVal1 * 10) + colorVal2;

  //Send the color data
  setValueSentOnAllFaces(data);
}
