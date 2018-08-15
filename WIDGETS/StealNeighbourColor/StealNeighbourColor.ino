/*
   steal neighbour by double-click

   An example showing how to communicate with surrounding blinks

   When single-clicked, a single blinks generate hero

*/
#include <Arduino.h>
#include <Serial.h>

ServicePortSerial Serial;

byte colorIndex;
bool isHeroHere = false;

enum MessageMode {
  EMPTY = 0,//null
  STEAL = 1,
  STOLEN = 2,
  NOHERO = 3,
};

// color random
Color colors[] = {
  RED,
  ORANGE,
  YELLOW,
  GREEN,
  BLUE,
  MAGENTA,
};

void setup() {
  // Blank
  isHeroHere = false;
  Serial.begin();
  Serial.println("Spread single Debug");
}


void loop() {

  if (buttonDoubleClicked()) {
    //change state
    isHeroHere = !isHeroHere;
    //reset values on all the faces
    colorIndex = rand(5);
    byte data = (colorIndex << 2) + EMPTY;
    setValueSentOnAllFaces(data);

    return;
  }

  //get it only once
  bool isStealNeighbour = buttonSingleClicked();

  FOREACH_FACE(f) {

    if ( !isValueReceivedOnFaceExpired( f ) ) {      // Have we seen an neighbor on this face recently?

      if (isStealNeighbour) {

        Serial.print("steal hero on face:");
        Serial.println(f);
        //send message
        byte data = (colorIndex << 2) + STEAL;
        setValueSentOnFace(data, f);

      } else if (didValueOnFaceChange( f )) {
        //only when message change then come in
        //receive message
        byte data = getLastValueReceivedOnFace( f ) & 3;
        colorIndex = (getLastValueReceivedOnFace( f ) >> 2) & 7;
        if (data == STEAL ) {
          //when hero is here
          if (isHeroHere) {
            //hero is no longer here
            isHeroHere = false;
            //report hero is no here
            byte data = (colorIndex << 2) + STOLEN;
            setValueSentOnFace(data, f);


            Serial.print("Hero steal by neighbor on face:");
            Serial.println(f);

          } else {
            //hero is not here, report no hero
            byte data = (colorIndex << 2) + NOHERO;
            setValueSentOnFace(data, f);

            Serial.print("send back no hero on face:");
            Serial.println(f);
          }

        } else if (data == STOLEN) {
          //steal hero from face f
          if (isHeroHere == false) {

            Serial.print("Steal hero on face:");
            Serial.println(f);

            //when no hero here, now hero is here
            isHeroHere = true;
            //stop sending steal message
            byte data = (colorIndex << 2) + 0;
            setValueSentOnFace(data, f);
          }

        } else if (data == NOHERO) {
          Serial.print("No hero on face:");
          Serial.println(f);
          //No hero on face f, stop sending steal message
          byte data = (colorIndex << 2) + EMPTY;
          setValueSentOnFace(data, f);
        }


      }

    } else {
      //when no face connects
      //empty message
      // color and state
      byte data = (colorIndex << 2) + EMPTY;
      setValueSentOnFace(data, f);
    }

  }

  //display hero
  if (isHeroHere) {
    setColor(colors[colorIndex]);
  } else {
    setColor(OFF);
  }



}




