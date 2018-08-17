#include <Arduino.h>
#include <Serial.h>

ServicePortSerial Serial;

#define IMPOSSIBLEINDEX FACE_COUNT
#define APPLE FACE_COUNT+1
#define EMPTYVALUE 0
#define MAX_DELATIME 250
#define MIN_DELATIME 150
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
  GAMEOVER  = 3,
  RESET = 4,
};

enum MessageMode{
  UPDATE  = 0,//UpdateIndex +UpdateMode + Mode
  DATA    = 1,//SnakeDir + SnakeLength + Mode
//  LENGTH  = 3,//no longer used
//  RESET
};

enum UpdateMode{
  NORMAL = 0,
  LENGTH_INCREASE = 1,
  LENGTH_DECREASE = 2,
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

uint32_t snakeFaceIncrement_ms = MAX_DELATIME;
//uint32_t nextSnakeFaceIncrement_ms = 0;

Timer faceIncreTimer, appleGenerateTimer;
const uint32_t appleGenerate_ms = MAX_DELATIME;
byte appleFace = IMPOSSIBLEINDEX;

Timer blinkTimer, animationTimer;
bool isBlinkOn;

int numMovesHere;

//store number of the snake on each face, 0 is null, from 1 - 6
uint32_t numSnakeArray[] = {
  0,0,0,0,0,0
};

void setup() {
  // put your setup code here, to run once:
  setColor(OFF);
  
  Serial.begin();
  Serial.println("SnakeV Debug");
  state = READY;
  initSnake();
  setColor(GREEN);
  setValueSentOnAllFaces(READY);
}

void setUpBlinkAnimation(){
  animationTimer.set(600);
  blinkTimer.set(100);
  isBlinkOn = false;
}

void loop(){
  // put your main code here, to run repeatedly:
    FOREACH_FACE(f){
      if(didValueOnFaceChange(f)){
        Serial.print("State:");
        Serial.print(state);
        Serial.print(": value on face");
        Serial.print(f);
        Serial.print(" change to ");
        Serial.println(getLastValueReceivedOnFace(f));
      }
    }
  
  if(state == READY){

    bool isGame = false;
    //When press button during ready, spawnSnake and tell other blinks to gaming
    if(buttonSingleClicked()){
      Serial.println("SnakeV spawn");
      spawnSnake();
      isGame = true;

    }else if(ifNeighborsAre(GAMEPLAY, GAMEPLAY, READY)){
      Serial.println("called to game");
      isGame = true; 
    }

    if(isGame){
      setColor(OFF);
      state = GAMEPLAY;
      setValueSentOnAllFaces(GAMEPLAY);
    }

  }
  else if(state == RESET){
   // setColor(BLUE);
    if(ifNeighborsAre(0,RESET,READY)){
      Serial.println("reset to ready");
      initSnake();
      state = READY;
      setColor(GREEN);
      setValueSentOnAllFaces(READY);
    }

  }else{

    //can be reset when gameplay and gameover
    if(buttonDoubleClicked() || ifNeighborsAre(RESET,0,0)){
      setColor(WHITE);
      state = RESET;

      setValueSentOnAllFaces(RESET);
      return;
    }

    if(state == GAMEPLAY){
      //if a neihbor is GAMEOVER, become GAMEOVER
      if(ifNeighborsAre(GAMEOVER,0,0)){
        setColor(RED);
        state = GAMEOVER;
        setValueSentOnAllFaces(GAMEOVER);
        Serial.println("called to gameover");
        return;
      }

      gameplayLoop();

    }

  }
  
}

void gameplayLoop(){

    //Serial.println(snakeFace);
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
          snakeLength--;
          snakeFaceIncrement_ms +=20;
          if(snakeFaceIncrement_ms > MAX_DELATIME){
                snakeFaceIncrement_ms = MAX_DELATIME;
          }
          //set up animation
          setUpBlinkAnimation();

          Serial.print("Length updates: ");
          Serial.println(snakeLength);
          //length already decrease by one
          updateSnakeArray(LENGTH_DECREASE);

        }
        
      }else{

        moveSnakeForward();

      }

    }else{
      
      //detect move forward message and length change messgae
      detectMessage();

      //generate apple
      generateApple();
    }

    if(state == GAMEPLAY){
      //draw face very frame during gameplay state
      drawFace();
    }
}

void destroyApple(){
  //if here is apple, apple disappear, reset appleface
      if(numSnakeArray[appleFace] == APPLE){
        //Serial.println("Apple destroy");
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

        //Serial.println("apple generate");
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

void initSnake(){
  snakeFaceIncrement_ms = MAX_DELATIME;
  numMovesHere = 0;
  isBlinkOn = true;
  snakeHue = 255;
  snakeFace = passToFace = passFromFace = appleFace = IMPOSSIBLEINDEX;
  snakeLength = 0;
  snakeDirection = CLOCKWISE;
  for(int i = 0;i < FACE_COUNT;i++){
    numSnakeArray[i] = 0;
  }
}

bool ifNeighborsAre(byte neededState, byte state1, byte state2){
  bool forReturn = (neededState>0)? false: true;
  FOREACH_FACE(f){
    //if here are connected blinks
    if(!isValueReceivedOnFaceExpired(f)){
      byte data = getLastValueReceivedOnFace(f);

      if(state1 > 0){
        if(data != state1){

          if(state2 > 0){

            if(data != state2){

              return false;
            }

          }else{

            return false;
          }
          
        }
      }

      if(neededState > 0 && data == neededState){
        forReturn = true;
      }
    }
      
  }

  return forReturn;
}

//(change dir randomly) 
//send message to the passto face, so that the next blinks will generate the head
void passSnake(){
  //check if need to change dir

  byte data = (snakeLength<<2)+(snakeDirection<<1)+/*((snakeHue/4)<<2)+*/DATA;
  Serial.print("Sent snake: Dir:");
  Serial.print(snakeDirection);
  Serial.print("  Length:");
  Serial.println(snakeLength);

  snakeFaceIncrement_ms = MAX_DELATIME;

  //send messgae to the next blinks through snakeface
  setValueSentOnFace(data,snakeFace);
}

//update snake position and display
void moveSnakeForward(){

  if( faceIncreTimer.isExpired() ) {

    numMovesHere ++;
    if(numMovesHere % FACE_COUNT == 0){
      snakeFaceIncrement_ms -= 20;
      if(snakeFaceIncrement_ms < MIN_DELATIME){
        snakeFaceIncrement_ms = MIN_DELATIME;
      }
      Serial.print("speed:");
      Serial.println(snakeFaceIncrement_ms);
    }

    //Serial.println("update");  
    updateSnakeArray(NORMAL);//it will call the next one to update
    faceIncreTimer.set(snakeFaceIncrement_ms);
  }

  drawFace();
}

void updateSnakeArray(byte updateMode){

  //check if snake have already been dead
  if(snakeLength < 1){
    Serial.println("snake is dead");
    state = GAMEOVER;
    setValueSentOnAllFaces(GAMEOVER);
    setColor(RED);
    return;
  }


    
  int startFace = getStartFace();
  
  byte newSnakeFace = snakeFace;
  int32_t newArray[] = {0,0,0,0,0,0};
  byte data = 0;
  int dir = getDir();
  //check head first
  for(int j = 0; j < 6; j++){

    int i = getFace(startFace + j * dir);

    //get the original index
    int originalNum = numSnakeArray[i];
    
      //max should equal
    //if there are body parts(1-6) APPLE - 6 + 1,0 means nothing there
    if(originalNum > 0 && originalNum < APPLE)
    {
      //max snake body index should equal to length, otherwise it should not go forward
      if(originalNum <= snakeLength){

        //bool for move forward or not
        bool isForward = true;

        //if the num was at the face the snake will pass to
        if(i == passToFace){

          Serial.print("On passTo, length:");
          Serial.print(snakeLength);
          Serial.print(", index:");
          Serial.println(originalNum);
          //do nothing, and if it's the tail, then end passing to the face
          if(originalNum == snakeLength 
            /*&& updateMode != LENGTH_INCREASE*/){
            //when the length is not increased
            Serial.println("---Tail leaves---");
            setValueSentOnFace(EMPTYVALUE,passToFace);
            passToFace = IMPOSSIBLEINDEX;

          }
          isForward = false;
        }

        if(isForward){//stay in current blinks

          //update the snake part to the next face
          int newNum = getFace(i + dir);

          //if it's the head
          if(originalNum == 1) {

            //update snakeFace
            newSnakeFace = newNum ;

            //if there is an apple the forward place 
            if(numSnakeArray[newNum] == APPLE){
              //eat apple
              destroyApple();
              Serial.println("Hits APPLE");

              //set up animation
              setUpBlinkAnimation();

              //max length is 6
              if(snakeLength < 6){

                if(updateMode == NORMAL){
                  Serial.print("Length increases:");
                  updateMode = LENGTH_INCREASE;
                  snakeLength++;
                  
                  Serial.println(snakeLength);
                }else{
                  Serial.println("increase during abnormal mode");
                }

              }
              //add flash visual feedbacks
            }
            
          }
          //go forward one step
          newArray[newNum] = numSnakeArray[i];
        }

        //When it was the tail and length increased
        if(originalNum == snakeLength - 1 && updateMode == LENGTH_INCREASE){
          Serial.println("Add snake tail");
          //add new tail at the old position and old tail will go forward
          newArray[i] = snakeLength;
        }
        
        //if the num was at the face the snake passes from
        if(i == passFromFace){

          //if the point here is the last one
          if(originalNum == snakeLength){

            Serial.print("close fromFace; tail in through face ");
            Serial.println(passFromFace);
            if(updateMode == LENGTH_DECREASE){
              //still send data when it decreases so that the next one delete the last tail
              data = ((snakeLength+1)<<3) + (updateMode<<1) + UPDATE;
            }
            setValueSentOnFace(EMPTYVALUE,passFromFace);

            passFromFace = IMPOSSIBLEINDEX;

          }else{

            //if it's not the last one, add the one after it here
            newArray[i] = originalNum + 1; 
            //send message to passFromFace, ask it to update array
            data = (newArray[i]<<3) + (updateMode<<1) + UPDATE;
            setValueSentOnFace(data,passFromFace);
          }
          
        }

      }

      
    }
  }
  
  snakeFace = newSnakeFace;
  
  if(appleFace != IMPOSSIBLEINDEX){
    newArray[appleFace] = APPLE;
  }

  if(data > 0){
    //only set data once at a time
      Serial.print("send update : mode:");
      Serial.print((data>>1)&3);
       Serial.print(" index:");
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


      byte mode = data&1;

      if(didValueOnFaceChange(f)){
        Serial.print("check face ");
        Serial.print(f);
        Serial.print(": mode ");
        Serial.println(mode);
      }


      if(data <= 4){
          continue;
      }

      if(mode == UPDATE){//1

        if(passToFace != IMPOSSIBLEINDEX ){
          //only when it calls the right index of the first body
          byte lastestGoneSnake = (data >> 3);
          byte snakeLengthChangeMode = ((data>>1)&3);
          
          if(numSnakeArray[passToFace] == lastestGoneSnake){

            Serial.print("delete body:");
            Serial.println(lastestGoneSnake);

            if(snakeLengthChangeMode == LENGTH_DECREASE){
              Serial.println("Received: length--;");
              //change snake length
              snakeLength --;
              snakeFaceIncrement_ms += 20;
              if(snakeFaceIncrement_ms > MAX_DELATIME){
                snakeFaceIncrement_ms = MAX_DELATIME;
              }
            }
            else if(snakeLengthChangeMode == LENGTH_INCREASE){
              //snakeLength++;
              Serial.println("Received: length++;");
              snakeLength++;
              
            }

            updateSnakeArray(snakeLengthChangeMode);
            
          }
          
        }

      }else if(mode == DATA){//2
        
        //only when there is no snake face in the piece, then receive new snake data
        //pass to face should not equal to pass from face
        if(snakeFace == IMPOSSIBLEINDEX && passToFace != f){
          Serial.print("Get snake: ");
          numMovesHere = 0;
          snakeFaceIncrement_ms = MAX_DELATIME;

          byte dir = (data >> 1) & 1;
          //get reverse direction
          snakeDirection = dir == COUNTER_CLOCKWISE? CLOCKWISE:COUNTER_CLOCKWISE;
          Serial.print("Dir:");
          Serial.print(dir);
          //get length
          snakeLength = (data >> 2);
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

          byte updateMode = 0;

          if(numSnakeArray[f] == APPLE){
            destroyApple();
            Serial.println("hit apple during transfering.");

            if(snakeLength < FACE_COUNT){
              snakeLength++;
              updateMode = LENGTH_INCREASE;
            }
            
          }

          
          //send message to passFromFace, ask it to update array
          byte data = (1<<3) +(updateMode<<1)+ UPDATE;
          setValueSentOnFace(data,passFromFace);
          Serial.print("send update : length");
          Serial.print((data>>1)&3);
           Serial.print(" & index");
          Serial.print(data>>3);
          Serial.print(" to passFromFace");
          Serial.println(passFromFace);
          
        }

      }
    }
  }
}


void drawFace(){
  // check if blinking animation happens
  if(animationTimer.isExpired()){
    isBlinkOn = true;
  }else{
    if(blinkTimer.isExpired()) {
      isBlinkOn = !isBlinkOn;
      blinkTimer.set(snakeFaceIncrement_ms);
    }
  }
  
  //draw snake using length and snakeface
  for(int i = 0; i < FACE_COUNT; i ++){
    byte brightness;
    byte hueAdjusted;

    byte snakeIndexOnFace = numSnakeArray[i];

    if(snakeIndexOnFace == APPLE) {
      //draw apple
      setFaceColor(i,RED);

    }else if(snakeIndexOnFace > 0){

      int distFromHead = snakeIndexOnFace - 1;

      hueAdjusted = 8 * ((32 + snakeHue - distFromHead) % 32);      
      brightness = 255 - (32 * distFromHead); // scale the brightness to 8 bits and dimmer based on distance from head
      

      setFaceColor(i, makeColorHSB( hueAdjusted, 255, brightness)); 

      if(snakeIndexOnFace == snakeLength){
        if(!isBlinkOn) {
          setFaceColor(i,WHITE);
        }
      }
      // if(isBlinkOn) {
      //   setFaceColor(i, makeColorHSB( hueAdjusted, 255, brightness));    
      // }
      // else {
      //   setFaceColor(i,WHITE);//?????
      // }

    }else {
      
        brightness = 0; // don't show past the tail of the snake
        setFaceColor(i,OFF);
    }

    

    
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

int getStartFace(){

    if(snakeFace != IMPOSSIBLEINDEX){
      return snakeFace;

    }else if(passToFace != IMPOSSIBLEINDEX){
      return passToFace;

    }else{
      Serial.println("no snake here");
      return 0;
    }

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
  
  
  //setStateToGameplayAndSendOutToAllConnected(FACE_COUNT);
}


