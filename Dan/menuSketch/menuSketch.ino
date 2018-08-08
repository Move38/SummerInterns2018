bool inMenuState;
bool choiceProgram;
bool choiceSleep;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // just deciding which loop to use
  if (inMenuState){
    menuLoop();
  } else {
    waitingLoop();
  }

  if (buttonLongPressed){//reverse the state
    inMenuState = !inMenuState;
  }
}

void waitingLoop(){
  
}

void menuLoop(){
  if(buttonSingleClicked()){
    
  }
  
  if(choiceProgram){
    
  } else if (choiceSleep){
    
  }
}

