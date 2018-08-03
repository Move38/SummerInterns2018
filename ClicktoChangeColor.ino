Color colors[] = {BLUE, RED, YELLOW, GREEN, CYAN, ORANGE };
byte currentColorIndex = rand(0);
byte faceIndex = 0;
byte faceStartIndex = 0;



void setup() {
  // put your setup code here, to run once:





}

void loop() {
  // put your main code here, to run repeatedly:
  if (buttonMultiClicked()) {
    setColor(WHITE);
  }

  if ( buttonSingleClicked() ) {

    currentColorIndex++;

    if (currentColorIndex >= COUNT_OF(colors)) {
      currentColorIndex = 0;
    }

  }


  // display color
  setColor( colors[currentColorIndex] );


}
