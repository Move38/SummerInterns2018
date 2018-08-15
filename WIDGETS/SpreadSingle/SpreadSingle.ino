/*
 * Spready once by single-click
 * 
 * An example showing how to communicate with surrounding blinks
 * 
 * When single-clicked, a single blinks color directly touching pieces
 *  
 */
#include <Arduino.h>
#include <Serial.h>

enum MessageMode{
  EMPTY = 0,//null
  REQUEST = 1,//request neighbor's color changing to its color
  ANSWER = 2,//answer its color has changed to what color
};

ServicePortSerial Serial;

byte colorIndex;
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
  colorIndex = rand(5);
  Color color = colors[colorIndex];
  setColor(color);
  Serial.begin();
  Serial.println("Spread single Debug");
}


void loop() {

  if(buttonDoubleClicked()){
    colorIndex = rand(5);
    Color color = colors[colorIndex];
    setColor(color);
    //reset the values on face
    setValueSentOnAllFaces(EMPTY);

    return;
  }

  bool isColorNeighbour = buttonSingleClicked();

  FOREACH_FACE(f) {

    if ( !isValueReceivedOnFaceExpired( f ) ) {      // Have we seen an neighbor on this face recently?

      if(isColorNeighbour){

        Serial.print("set Color change value on face:");
        Serial.println(f);
        //send message
        byte dataForRequest = (colorIndex<<2)+REQUEST;
        setValueSentOnFace(dataForRequest,f);

      }else {

        //receive request message
        byte dataReceived = getLastValueReceivedOnFace( f );
        //only when message change then come in
        byte mode = dataReceived&3;

        if(mode == REQUEST){
          //get request color index
          int requestedColorIndex = (dataReceived>>2);

          colorIndex = requestedColorIndex;

          Serial.print("Color changed by neighbor on face:");
          Serial.print(f);
          Serial.print(" to color:");
          Serial.println(colorIndex);

          //display color
          setColor(colors[colorIndex]);

          //send the answer so that the sender won't keep the value at the face
          byte dataForAswer = (colorIndex<<2)+ANSWER;
          setValueSentOnFace(dataForAswer,f);

        }else if(mode == ANSWER){
          Serial.print("Answer by neighbor on face:");
          Serial.print(f);
          Serial.print(" for color:");
          Serial.println(colorIndex);
          //get request color index
          int answeredColorIndex = (dataReceived>>2);
          //when color is the same, reset value on the face
          if(answeredColorIndex == colorIndex){
            Serial.print("Answer right so value reset on face:");
            Serial.println(f);
            setValueSentOnFace(EMPTY,f);
          }
        }

      }

    }else{
      //when no face connects
      //empty message
      setValueSentOnFace(EMPTY,f);
    }

  }
  
}




