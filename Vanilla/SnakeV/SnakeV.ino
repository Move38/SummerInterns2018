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
 // LENGTH  = 3,//no longer used
//  RESET
};

enum Direction{
  COUNTER_CLOCKWISE,
  CLOCKWISE
};

byte state = READY;

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

Timer faceIncreTimer, appleGenerateTimer;
uint32_t appleGenerate_ms = 250;
byte appleFace = IMPOSSIBLEINDEX;


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

  // FOREACH_FACE(f){
  //   if(didValueOnFaceChange(f)){
  //     Serial.print("On face ");
  //     Serial.print(f);
  //     Serial.print(" value changes to ");
  //     Serial.println(getLastValueReceivedOnFace(f));
      
  //   }
  // }
  
  
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
    
    //if snake head is here
    if(snakeFace != IMPOSSIBLEINDEX){

      //when snake head just passed from the face, don't pass again
      if(buttonSingleClicked() && snakeFace != passFromFace){
        Serial.println("gameplay: click");

        if(!isValueReceivedOnFaceExpired(snakeFace)){
          //pass snake when press button and the head is facing other blinks
          passToFace = snakeFace;//then will never do moveSnakeForward every frame
          passSnake();
          snakeFace = IMPOSSIBLEINDEX;
          appleGenerateTimer.set(appleGenerate_ms*rand(3));

        }else{
          // Serial.print("length --:");
          // Serial.println(snakeFace);
          // snakeLength --;//tell other's as well
          
          // if(passFromFace != IMPOSSIBLEINDEX){
          //   byte data = (snakeLength<<2)+LENGTH;
          //   setValueSentOnFace(data,passFromFace);
          // }

        }
        
        //updateSnakeArray();
        return;
      }

      moveSnakeForward();

      

      
    }else{
      //detect move forward message and length change messgae
      detectMessage();

      //generate apple
      generateApple();
    }

    //draw face very frame during gameplay state
    drawFace();
    
  }
  else if(state == GAMEOVER){
    
  }
  
}


void destroyApple(){
  //if here is apple, apple disappear, reset appleface
      if(numSnakeArray[appleFace] == APPLE){
        Serial.println("Apple destroy");
        numSnakeArray[appleFace] = 0;

        //reset timer to generate timer
        appleGenerateTimer.set(appleGenerate_ms);
      }
      //reset appleFace
      appleFace = IMPOSSIBLEINDEX;
}

void generateApple(){

  if(appleGenerateTimer.isExpired()){
    
    if(appleFace != IMPOSSIBLEINDEX){
      destroyApple();
      
    }else{

      int randFace = rand(5);//random 0 - 5

      if(numSnakeArray[randFace] == 0){

        Serial.println("apple generate");
        numSnakeArray[randFace] = APPLE;
        appleFace = randFace;
        //set timer for self-destroy
        appleGenerateTimer.set(appleGenerate_ms*100);

      }else{

        appleGenerateTimer.set(appleGenerate_ms);

      }
      
    }

  }

}


void reset(){
  setColor(GREEN);

  state = READY;

  appleFace = IMPOSSIBLEINDEX;

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
        // Serial.print("set new value(state):");
        // Serial.print(state);
        // Serial.print(" on face ");
        // Serial.println(f);
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
        Serial.println("Gaming");

        setStateToGameplayAndSendOutToAllConnected(f);
      }
    }
      
  }
}

//(change dir randomly) 
//send message to the passto face, so that the next blinks will generate the head
void passSnake(){
  //check if need to change dir

  byte data = (snakeLength<<3)+(snakeDirection<<2)+/*((snakeHue/4)<<2)+*/DATA;
  Serial.print("Snake passed on face ");
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
    setFaceColor(WHITE,snakeFace);
    state = GAMEOVER;
    return;
  }

   int32_t newArray[] = {0,0,0,0,0,0};
    
    int dir = getDir();
    
    //default snake length didn't increase
    bool isSnakeLengthIncreased = false;
    byte newSnakeFace = snakeFace;
    
    int startFace = 0;
    if(snakeFace != IMPOSSIBLEINDEX){
      startFace = snakeFace;
    }else if(passToFace != IMPOSSIBLEINDEX){
      startFace = passToFace;
      //Serial.println("start at passto");
    }else{
      Serial.println("no snake here");
      return;
    }
    
    byte data = 0;

    //check head first
    for(int j = 0; j < 6; j++){

      int i = startFace + j * dir;
      if(i < 0){
        i+=6;
      }
      i%=6;
      //get the original index
      int originalNum = numSnakeArray[i];
      
        //max should equal
      //if there are body parts(1-6) APPLE - 6 + 1
      if(originalNum > 0 && originalNum < APPLE)
      {
        //0 means nothing there
        //Serial.print(i);
        //max should equal to length
        if(originalNum <= snakeLength){

          //bool for move forward or not
          bool isForward = true;

          //if the num was at the face the snake passes from
          if(i == passFromFace){

            //if the point here is the last one
            if(originalNum == snakeLength){

              Serial.print("close fromFace; tail in through face ");
              Serial.println(passFromFace);

              //setValueSentOnFace(0,passFromFace);
              passFromFace = IMPOSSIBLEINDEX;

            }else{

              //if it's not the last one, add the one after it here
              newArray[i] = originalNum + 1; 
              //send message to passFromFace, ask it to update array
              data += (newArray[i]<<3) + (0<<2) + UPDATE;
              
            }
            
          }

          //if the num was at the face the snake will pass to
          if(i == passToFace){
            Serial.print("when passTo length:");
            Serial.print(snakeLength);
            Serial.print(" index:");
            Serial.println(originalNum);
            //do nothing, and if it's the tail, then end passing to the face
            if(originalNum == snakeLength){
              Serial.println("---Tail leaves this tile---");
              
              passToFace = IMPOSSIBLEINDEX;
            }
            isForward = false;
          }

          //When it was the tail and length increased
          if(originalNum == snakeLength && isSnakeLengthIncreased){
            Serial.println("Add snake tail");
            //add a snake part here
            newArray[i] = snakeLength + 1;
          }

          if(isForward){//stay in current blinks

            int next = i + dir;
            if(next < 0){
              next += 6;
            }
            //update the snake part to the next face
            int newNum = next % 6;

            //if it's the head
            if(originalNum == 1) {

              //update snakeFace
              newSnakeFace = newNum ;

              //if there is an apple the forward place 
              if(numSnakeArray[newNum] == APPLE){
                //eat apple
                destroyApple();
                Serial.print("Hits APPLE, ");
                //max length is 6
                if(snakeLength < 6){

                  isSnakeLengthIncreased = true;

                   //send messge to from and to
                   if(passFromFace != IMPOSSIBLEINDEX){
                     //update length-----------------------------------------
                     data += (1<<2);
                   }

                   
                    
                }
                //add flash visual feedbacks
              }
              
            }
            //go forward one step
            newArray[newNum] = numSnakeArray[i];
          }

        }

        
      }
    }
    
    snakeFace = newSnakeFace;
    
    if(appleFace != IMPOSSIBLEINDEX){
      newArray[appleFace] = APPLE;
    }

    //when snake Length Increase, change snakelength
    if(isSnakeLengthIncreased){

      snakeLength++;
      //send messgae to the next blinks through snakeface
      Serial.print("Length updates: ");
      Serial.println(snakeLength);
    }

    if(data > 0){
      //only set data once at a time
        setValueSentOnFace(data,passFromFace);
 
        Serial.print("send update : length");
        Serial.print((data&4)>>2);
         Serial.print(" & index");
        Serial.print(data>>3);
        Serial.print(" to passFromFace");
        Serial.println(passFromFace);
    }

    memcpy( numSnakeArray, newArray, 6*sizeof(uint32_t) );

}

void detectMessage(){

  FOREACH_FACE(f) {

    if(!isValueReceivedOnFaceExpired(f)){

      byte data = getLastValueReceivedOnFace(f);

      if(data < 4){
          return;
      }

      if(didValueOnFaceChange(f)){
        Serial.print("Changes On face ");
        Serial.println(f);
      }

      MessageMode mode = static_cast<MessageMode>(data&3);
      //Serial.print("receive message mode ");
      //Serial.println(data&3);

      if(mode == UPDATE){//1

        if(passToFace != IMPOSSIBLEINDEX ){
          //only when it calls the right index of the first body
          byte lastestGoneSnake = (data >> 3);
          bool isSnakeLengthIncreased = ((data&4) >> 2) > 0;
          
          if(numSnakeArray[passToFace] == lastestGoneSnake){

            Serial.print("delete body:");
            Serial.println(lastestGoneSnake);
            
            if(isSnakeLengthIncreased){
              snakeLength++;
              Serial.println("snake length++, add a tail");
              AddTail();
            }
            updateSnakeArray();
            //setValueSentOnFace(passToFace);
          }
          //drawFace();
          
        }

      }else if(mode == DATA){//2
        //println("")
        //only when there is no snake face in the piece, then receive new snake data
        //pass to face should not equal to pass from face
        if(snakeFace == IMPOSSIBLEINDEX && passToFace != f){
          Serial.print("Get data for the snake: ");
          Direction dir = static_cast<Direction>((data & 4) >> 2);
          //get direction
          snakeDirection = dir;
          Serial.print("Dir:");
          Serial.print(dir);
          //get length
          snakeLength = (data >> 3);
          Serial.print("  Length:");
          Serial.print(snakeLength);
           //get hue
           //snakeHue = 4 * ((data & 31) >> 2);
           // Serial.print("  snakeHue:");
           // Serial.print(snakeHue);

          snakeFace = f;
          numSnakeArray[snakeFace] = 1;
          faceIncreTimer.set(snakeFaceIncrement_ms); 
          Serial.print("  snakeFace:");
          Serial.println(snakeFace);

          passFromFace = f;

          byte isSnakeLengthIncreased = 0;

          if(numSnakeArray[f] == APPLE){
            destroyApple();
            Serial.println("hit apple during transfering.");

            if(snakeLength < FACE_COUNT){
              snakeLength++;
              isSnakeLengthIncreased = 1;
            }
            
          }

          //updateSnakeArray();
          //send message to passFromFace, ask it to update array
          byte data = (1<<3) +(isSnakeLengthIncreased<<2)+ UPDATE;
          setValueSentOnFace(data,passFromFace);
          Serial.print("send update : length");
          Serial.print((data&4)>>2);
           Serial.print(" & index");
          Serial.print(data>>3);
          Serial.print(" to passFromFace");
          Serial.println(passFromFace);
          
        }

      }
    }
  }
}


void AddTail(){
  int passToNum = numSnakeArray[passToFace];
  int tailIndex = snakeLength;
  int tailFace = getFace(passToFace - (snakeLength - passToNum) * getDir());
  numSnakeArray[tailFace] = tailIndex;
  Serial.print("Add tail at face:");
  Serial.println(tailFace);
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

int getDir(){
    if(snakeDirection == COUNTER_CLOCKWISE){
      return -1;
    }else{
      return 1;
    }
}

int getFace(int i){
  if(i < 0){
    i+=6;
  }
  i%=6;
  return i;
}

void spawnSnake(){
  //do spawn snake
  snakeLength = 3;
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
