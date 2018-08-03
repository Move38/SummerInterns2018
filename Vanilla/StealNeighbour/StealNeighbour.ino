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
bool isHeroHere;
// color random
Color colors[] = {    
  WHITE,            
  RED,                
}; 


void setup() {
  // Blank
  isHeroHere = false;
  Serial.begin();
  Serial.println("Spread single Debug");
}


void loop() {

  if(buttonSingleClicked()){
    isHeroHere = true;
    return;
  }

  bool isStealNeighbour = buttonDoubleClicked();

  FOREACH_FACE(f) {

    if ( !isValueReceivedOnFaceExpired( f ) ) {      // Have we seen an neighbor on this face recently?

      if(isStealNeighbour){
        
        Serial.print("steal hero on face:");
        Serial.println(f);
        //send message
        setValueSentOnFace(1,f);

      }else if(didValueOnFaceChange(f)){
        //only when message change then come in
        //receive message
        byte data = getLastValueReceivedOnFace( f );
        if(data == 1 ){
          if(isHeroHere){

            isHeroHere = false;
            setValueSentOnFace(2,f);

            Serial.print("Hero steal by neighbor on face:");
            Serial.println(f);

          }else{

            setValueSentOnFace(3,f);

            Serial.print("send back no hero on face:");
            Serial.println(f);
          }
          
        }else if(data == 2){

          if(isHeroHere == false){
            isHeroHere = true;
            Serial.print("Steal hero on face:");
            Serial.println(f);
            setValueSentOnAllFaces(0);
          }

        }else if(data == 3){
           Serial.print("No hero on face:");
          Serial.println(f);
          setValueSentOnFace(0,f);
        }

        
      }

    }else{
      //when no face connects
      //empty message
      setValueSentOnFace(0,f);
    }
  
  }

  //display hero
  if(isHeroHere){
    setColor(RED);
  }else{
    setColor(WHITE);
  }

  

}




