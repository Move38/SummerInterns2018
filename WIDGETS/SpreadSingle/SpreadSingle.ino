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

ServicePortSerial Serial;

byte colorIndex;
// color random
Color colors[] = {    
  RED,            
  YELLOW,         
  GREEN,          
  CYAN,           
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

  if(buttonSingleClicked()){
    colorIndex = rand(5);
    Color color = colors[colorIndex];
    setColor(color);
    setValueSentOnAllFaces(0);
    return;
  }

  bool isColorNeighbour = buttonDoubleClicked();

  FOREACH_FACE(f) {

    if ( !isValueReceivedOnFaceExpired( f ) ) {      // Have we seen an neighbor on this face recently?

      if(isColorNeighbour){
        Serial.print("set Color change value on face:");
        Serial.println(f);
        //send message
        setValueSentOnFace(colorIndex + 1,f);

      }else if(didValueOnFaceChange(f)){
        //only when message change then come in
        //receive message
        byte data = getLastValueReceivedOnFace( f );
        if(data > 0){
          colorIndex = data - 1;

          Serial.print("Color changed by neighbor on face:");
          Serial.print(f);
          Serial.print(" to color:");
          Serial.println(colorIndex);
          setColor(colors[colorIndex]);
        }

        
      }

    }else{
      //when no face connects
      //empty message
      setValueSentOnFace(0,f);
    }
  
  }

  

  

}




