/*
 * steal neighbour by double-click
 * 
 * An example showing how to communicate with surrounding blinks
 * 
 * When single-clicked, a single blinks generate hero
 *  
 */
#include <Arduino.h>
#include <Serial.h>

ServicePortSerial Serial;

byte colorIndex;
bool isHeroHere = false;

enum MessageMode{
  EMPTY = 0,//null
  STEAL = 1,
  STOLEN = 2,
  NOHERO = 3,
};

void setup() {
  // Blank
  isHeroHere = false;
  Serial.begin();
  Serial.println("Spread single Debug");
}


void loop() {

  if(buttonSingleClicked()){
    //change state
    if(isHeroHere){
      isHeroHere = false;
    }else{
      isHeroHere = true;
    }
    //reset values on all the faces
    setValueSentOnAllFaces(EMPTY);

    return;
  }

  //get it only once
  bool isStealNeighbour = buttonDoubleClicked();

  FOREACH_FACE(f) {

    if ( !isValueReceivedOnFaceExpired( f ) ) {      // Have we seen an neighbor on this face recently?

      if(isStealNeighbour){
        
        Serial.print("steal hero on face:");
        Serial.println(f);
        //send message
        setValueSentOnFace(STEAL,f);

      }else if(didValueOnFaceChange( f )){
        //only when message change then come in
        //receive message
        byte data = getLastValueReceivedOnFace( f );
        if(data == STEAL ){
          //when hero is here
          if(isHeroHere){
            //hero is no longer here
            isHeroHere = false;
            //report hero is no here
            setValueSentOnFace(STOLEN,f);

            Serial.print("Hero steal by neighbor on face:");
            Serial.println(f);

          }else{
            //hero is not here, report no hero
            setValueSentOnFace(NOHERO,f);

            Serial.print("send back no hero on face:");
            Serial.println(f);
          }
          
        }else if(data == STOLEN){
          //steal hero from face f
          if(isHeroHere == false){
            
            Serial.print("Steal hero on face:");
            Serial.println(f);

            //when no hero here, now hero is here
            isHeroHere = true;
            //stop sending steal message
            setValueSentOnAllFaces(0);
          }

        }else if(data == NOHERO){
           Serial.print("No hero on face:");
          Serial.println(f);
          //No hero on face f, stop sending steal message
          setValueSentOnFace(EMPTY,f);
        }

        
      }

    }else{
      //when no face connects
      //empty message
      setValueSentOnFace(EMPTY,f);
    }
  
  }

  //display hero
  if(isHeroHere){
    setColor(RED);
  }else{
    setColor(OFF);
  }

  

}




