#include <Arduino.h>
#include <Serial.h>

ServicePortSerial Serial;

#define RED     makeColorRGB(255,   0,   0)
#define GREEN   makeColorRGB(  0, 255,   0)
#define IMPOSSIBLEINDEX FACE_COUNT
#define APPLE FACE_COUNT+1
#define EMPTYVALUE 0
// Information to convey over message
// Game Mode (1-3) - 2 bits & Message mode (1-3)
// Passing of snake
//  snake hue (0-31) - 3 bits 
//  snake length (3-31) - 5 bits? & 111 11 >> 2
//  snake direction(0-1) & 1 000 00 >> 5
// pass information in a byte
//
// 0 - 3 bits for length(maximum is 6 now), 1 for dir, 6 - 11 bits for hue, 2 bits for message mode
// i.e.
// 00011 1 010000 01 = length of 3, hue of 16, clockwise and message mode of UPDATE

enum State{
  READY     = 1,  //no snake spawn yet. if long pressed, spawn a snake, set every connected pieces to gameplay
  GAMEPLAY  = 2,  //a snake has spawned and other pieces in gameplay can get the message
  GAMEOVER  = 3
};

enum MessageMode{
  UPDATE  = 1,
  DATA    = 2,
  LENGTH  = 3,
//  RESET
};

enum Direction{
  COUNTER_CLOCKWISE,
  CLOCKWISE
};

byte state = READY;
bool isSnakeHere = false;
//bool isSnakeHeadHere = false, isSnakeTailHere = false;
byte snakeHue = 255;
//set to an impossible number as default
byte snakeFace = IMPOSSIBLEINDEX;//head face: 0 - 5, 6 is default
byte passToFace = IMPOSSIBLEINDEX;//face that the snake will pass to:
                                  // 0 - 5, 6 is default
byte passFromFace = IMPOSSIBLEINDEX;//face that the snake will pass from:
                                   // 0 - 5, 6 is default
byte snakeLength = 0;// maximun is 6
byte snakeDirection = CLOCKWISE;

uint32_t snakeFaceIncrement_ms = 250;
uint32_t nextSnakeFaceIncrement_ms = 0;

Timer faceIncreTimer;

//store number of the snake on each face, 0 is null, from 1 - 6
uint32_t numSnakeArray[] = {
  0,0,0,0,0,0
};

void setup() {
  // put your setup code here, to run once:
  setColor(OFF);
  
  Serial.begin();
  Serial.println("SnakeV Debug");
  reset();
}


//check if player wants to reset all the pieces
bool checkReset(){
  bool isReset = false;;
  //double click to reset
  if(buttonDoubleClicked()){
    Serial.println("double pressed to reset");
    isReset = true;
  }else{

    FOREACH_FACE(f){
    //if here are connected blinks send ready
      if(!isValueReceivedOnFaceExpired(f) /*&& didValueOnFaceChange(f)*/){
        if(getLastValueReceivedOnFace(f) == READY && state != READY){
          Serial.println("Called to reset to ready");
          isReset = true;
        }
      }
    }
  }

  return isReset;
}

void loop(){
  // put your main code here, to run repeatedly:

  //reset is available at any time
  if(checkReset()){
    reset();
    //sendStateOnConnectedFace(FACE_COUNT);
    return;
  }

  FOREACH_FACE(f){
    if(didValueOnFaceChange(f)){
      Serial.print("On face ");
      Serial.print(f);
      Serial.print(" value changes to ");
      Serial.println(getLastValueReceivedOnFace(f));
      
    }
  }
  
  
  if(state == READY){

    //When press button during ready, spawnSnake and tell other blinks to gaming
    if(buttonLongPressed()){
      Serial.println("SnakeV spawn");
      spawnSnake();
    }else{
      //check if other blinks send begin message
      checkBeginMessage();
    }

  }
  else if(state == GAMEPLAY){
    //Serial.println(snakeFace);
    //if snake head is here
    if(snakeFace != IMPOSSIBLEINDEX){
      

      moveSnakeForward();
      
      if(buttonSingleClicked()){
        Serial.println("gameplay: single click");

        if(!isValueReceivedOnFaceExpired(snakeFace)){

          Serial.print("snake head passed to face");
          Serial.println(snakeFace);

          //pass snake when press button and the head is facing other blinks
          passToFace = snakeFace;//then will never do moveSnakeForward every frame
          passSnake();
          snakeFace = IMPOSSIBLEINDEX;
          //update snake position according to the dir and passToFace
          //moveSnakeForward();
          
        }else{
          Serial.print("Wrong click on ");
          Serial.println(snakeFace);
          //snakeLength --;//tell other's as well
        }
        
      }
      
    }else{
      //detect move forward message and length change messgae
      detectMessage();
    }

    //draw face very frame during gameplay state
    drawFace();
    
  }
  else if(state == GAMEOVER){
    
  }
  
}

void reset(){
  setColor(GREEN);

  state = READY;

  isSnakeHere = false;
  snakeHue = 255;
  snakeFace = passToFace = passFromFace = IMPOSSIBLEINDEX;
  snakeLength = 0;
  snakeDirection = CLOCKWISE;
  for(int i = 0;i < FACE_COUNT;i++){
    numSnakeArray[i] = 0;
  }
  setValueSentOnAllFaces(EMPTYVALUE);
}

void sendStateOnConnectedFace(byte face){
  Serial.println("clear value on faces");
  setValueSentOnAllFaces(EMPTYVALUE);
  FOREACH_FACE(f){
    //if here are connected blinks
      if(!isValueReceivedOnFaceExpired(f) && f != face){
        Serial.print("set new value(state):");
        Serial.print(state);
        Serial.print(" on face ");
        Serial.println(f);
        setValueSentOnFace(state,f);
      }
    }
}

//check begin messaga, only called during ready
void checkBeginMessage(){
  FOREACH_FACE(f){
    //if here are connected blinks
    //Serial.println()
    if(!isValueReceivedOnFaceExpired(f)){
      byte data = getLastValueReceivedOnFace(f);
      
      if(data == GAMEPLAY && state != GAMEPLAY){
        Serial.println("This tile are called to gaming.");
        //if(didValueOnFaceChange(f)){
          //Serial.println("The message changed");
          setStateToGameplayAndSendOutToAllConnected(f);
        //}
        //return;//once change, don't need to check next
      }
    }
      
  }
}

//(change dir randomly) 
//send message to the passto face, so that the next blinks will generate the head
void passSnake(){
  //check if need to change dir

  byte data = (snakeLength<<3)+(snakeDirection<<2)+/*((snakeHue/4)<<2)+*/DATA;
  Serial.print("Snake passing on face ");
  Serial.print(snakeFace);
  Serial.print(" with data ");
  Serial.print(data);  
  Serial.print(": Direction:");
  Serial.print(snakeDirection);
  Serial.print("  snakeLength:");
  Serial.println(snakeLength);
  //send messgae to the next blinks through snakeface
  setValueSentOnFace(data,snakeFace);
}

//update snake position and display
void moveSnakeForward(){

  //uint32_t now = millis();
  if( faceIncreTimer.isExpired() ) {
    //Serial.println("update");  
    updateSnakeArray();//it will call the next one to update
    //nextSnakeFaceIncrement_ms = now + snakeFaceIncrement_ms;
    faceIncreTimer.set(snakeFaceIncrement_ms);
  }

  drawFace();
}

void updateSnakeArray(){
  if(snakeLength <= 0){
    Serial.println("snake is dead");
    //reset();
    //setValueSentOnAllFaces(RESET);
    return;
  }

   int32_t newArray[] = {0,0,0,0,0,0};
    
    int dir = 1;     
    if(snakeDirection == COUNTER_CLOCKWISE){
      dir = -1;
    }
    
    for(int i = 0; i < 6; i++){

      //get the original index
      int originalNum = numSnakeArray[i];

      //if there are body parts(1-6) APPLE - 6 + 1
      if(originalNum > 0 && originalNum < APPLE){//0 means nothing there
        

        //max should equal to length
        if(originalNum <= snakeLength){
          //bool for move forward or not
          bool isForward;

          //if the num was at the face the snake passes from
          if(i == passFromFace){

            //if the point here is the last one
            if(originalNum == snakeLength){

              Serial.print("close fromFace; tail in through face ");
              Serial.println(passFromFace);


              passFromFace = IMPOSSIBLEINDEX;

              isForward = false;
            }else{
              //if it's not the last one, add the one after it here
              newArray[i] = originalNum + 1; 
              
              //send message to passFromFace, ask it to update array
              byte data = (newArray[i]<<2) + UPDATE;
              setValueSentOnFace(data,passFromFace);
 
              Serial.print("send update cmd: ");
              Serial.print(data);
              Serial.print(" to passFromFace");
              Serial.println(passFromFace);

              isForward = true;
            }
            
          }

          //if the num was at the face the snake will pass to
          if(i == passToFace){
            //do nothing, and if it's the tail, then end passing to the face
            if(originalNum == snakeLength){
              Serial.print("Tail leaves this tile through face ");
              Serial.println(passToFace);
              //Serial.println("reset passToFace to IMPOSSIBLEINDEX");
              passToFace = IMPOSSIBLEINDEX;
              //snakeFace = IMPOSSIBLEINDEX;
              isSnakeHere = false;
            }
            isForward = false;
          }else{
            isForward = true;
          }


          if(isForward){//stay in current blinks

            //update the snake part to the next face
            int newNum = i + dir;

            if(newNum > 5) newNum = 0;//detect to in the future
            if(newNum < 0) newNum = 5;
          
            // Serial.print("Snake part ");
            // Serial.print(originalNum);
            // Serial.print(" goes from ");
            // Serial.print(i);
            // Serial.print(" to ");
            // Serial.println(newNum);

            //if it's the head part
            if(originalNum == 1) {
              //update snakeFace
              snakeFace = newNum ;
              // Serial.print("snakeFace updates to ");
              // Serial.println(snakeFace);

              //if there is an apple the forward place 
              if(numSnakeArray[newNum] == APPLE){

                //max length is 6
                if(snakeLength <= 6){
                   snakeLength++;
                   //send messge to from and to
                   if(passFromFace != IMPOSSIBLEINDEX){
                     //update length-----------------------------------------
                     byte data = (snakeLength<<2)+LENGTH;
                     //send messgae to the next blinks through snakeface
                     //Serial.print("snake hits apple, its length updates to ");
                     //Serial.println(snakeLength);
                     setValueSentOnFace(data,passFromFace);
                   }
                }
                //add flash visual feedbacks
              }
              
            }
            newArray[newNum] = numSnakeArray[i];
          }
        
          
        }

        
      }else{
        //Serial.println(rand(10));
        //here is not snake body
        //randomly generate an apple here
       // if(originalNum != APPLE){
       //   Serial.print("generate apple at a empty spot:");
       //   Serial.println(i);
       //   int randNumber = rand(60);
       //   if(randNumber == 2){
       //     newArray[i] = APPLE;
       //   }
       // }
      }
    }

    memcpy( numSnakeArray, newArray, 6*sizeof(uint32_t) );
}

void detectMessage(){
  FOREACH_FACE(f) {
    if(!isValueReceivedOnFaceExpired(f)){

      byte data = getLastValueReceivedOnFace(f);
      // Serial.print("receive message ");
      // Serial.print(data);
      // Serial.print(" on face ");
      // Serial.println(f);

      if(data < 4){
          return;
      }

      
      MessageMode mode = static_cast<MessageMode>(data&3);
      // Serial.print("receive message mode ");
      // Serial.println(data&3);

      if(mode == UPDATE){//1
        if(passToFace != IMPOSSIBLEINDEX ){
          //only when it calls the right index of the first body
          byte lastestGoneSnake = (data >> 2);

          if(numSnakeArray[passToFace] == lastestGoneSnake){
            Serial.print("delete body index:");
            Serial.print(lastestGoneSnake);
            Serial.println(" on the old pieces");

            updateSnakeArray();
            //setValueSentOnFace(passToFace);????????????????
          }
          //drawFace();
          
        }

      }else if(mode == DATA){//2

      //only when there is no snake face in the piece, then receive new snake data
        //pass to face should not equal to pass from face
        if(snakeFace == IMPOSSIBLEINDEX && passToFace != f){
          Serial.println("new blink get data for the snake");
          Direction dir = static_cast<Direction>((data & 4) >> 2);
          //get direction
          snakeDirection = dir;
          Serial.print("Direction:");
          Serial.print(dir);
          //get length
           snakeLength = (data >> 3);
           Serial.print("  snakeLength:");
           Serial.print(snakeLength);
           //get hue
           //snakeHue = 4 * ((data & 31) >> 2);
           // Serial.print("  snakeHue:");
           // Serial.print(snakeHue);

           snakeFace = f;
           passFromFace = f;
           numSnakeArray[f] = 1;
           isSnakeHere = true;
          
          //send message to passFromFace, ask it to update array
          byte data = (1<<2) + UPDATE;
          setValueSentOnFace(data,passFromFace);

           Serial.print("  snakeFace:");
           Serial.println(snakeFace);
           faceIncreTimer.set(snakeFaceIncrement_ms); 
        }
        
           
      }else if(mode == LENGTH){//3

        //only have length
        int newLength = data >> 2;
        snakeLength = newLength;
        
        //if tail is here
        if(passFromFace < 0){
          updateLength();
        }else{
          //tell the next to find tail and change length
          setValueSentOnFace(data,passFromFace);
        }
        
      }
    }
  }
}

void updateLength(){
  bool addANewOne = true;
  int lastFace = -1;
   for(int i = 0; i < 6; i++){
      int num = numSnakeArray[i];
      if(num + 1 > snakeLength - 1){
        //if exceeds the maxlength, reset it to null and update
        //Serial.println("Delete one at tail");
        addANewOne = false;
        numSnakeArray[i] = -1;
      }else if(num + 1 == snakeLength - 1){
        lastFace = i;
      }
   }

   if(addANewOne){
    
    int dir = 1;     
    if(snakeDirection == COUNTER_CLOCKWISE){
      dir = -1;
    }
    //here is snake body
     int newNum = lastFace + dir;
    if(newNum > 5) newNum = 0;//detect to in the future
    if(newNum < 0) newNum = 5;
    
    int num = numSnakeArray[newNum];
    if(num < 0){
      //if here is empty or apple
      //Serial.println("Add one after tail");
      numSnakeArray[newNum] = snakeLength - 1;
    }
    
   }
  updateSnakeArray();
  
   
}


void drawFace(){
  //draw snake using length and snakeface
  // clear the display buffer
  setColor(OFF);

  for(int i = 0; i < FACE_COUNT; i ++){
    byte brightness;
    byte hueAdjusted;

    byte snakeIndexOnFace = numSnakeArray[i];

    if(snakeIndexOnFace == APPLE) {
      //draw apple
      hueAdjusted = 0;
      brightness = 255;

    }else if(snakeIndexOnFace > 0){

      int distFromHead = snakeIndexOnFace - 1;

      hueAdjusted = 8 * ((32 + snakeHue - distFromHead) % 32);      
      brightness = 255 - (32 * distFromHead); // scale the brightness to 8 bits and dimmer based on distance from head
      
    }else {
      
        brightness = 0; // don't show past the tail of the snake
      
    }
    setFaceColor(i, makeColorHSB( hueAdjusted, 255, brightness));
  }
    
}

void spawnSnake(){
  //do spawn snake
  snakeLength = 3;
  isSnakeHere = true;
  snakeDirection = CLOCKWISE;
  snakeFace = 2;//default head is 0
  passFromFace = passToFace = IMPOSSIBLEINDEX;
  numSnakeArray[0] = 3;//the first part of the snake - head, begins at 1
  numSnakeArray[1] = 2;
  numSnakeArray[2] = 1;
  
  setStateToGameplayAndSendOutToAllConnected(FACE_COUNT);
}

//send out game play state and init timer
void setStateToGameplayAndSendOutToAllConnected(byte face){
  setColor(OFF);
  //tell all the other blinks to change state to gameplay 
  state = GAMEPLAY;
  sendStateOnConnectedFace(face);
  faceIncreTimer.set(snakeFaceIncrement_ms);
}
