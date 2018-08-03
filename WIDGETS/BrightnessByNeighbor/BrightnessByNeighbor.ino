/*
 * Brightness By Number/Neighbor
 * 
 * An example showing how to count the number of neighbors.
 * 
 * Each Blink displays a brightness based on the number of neighbors around it.
 *  
 */

//human eyes have difficulties recognzing when brightness are changing linearly
//have a array for different number of neighour
int brightnessArray[] = {
  255,//no neighbour
  155,//1 neighbour
  100,//2 neighbour
  60,//3 neighbour
  30,//4 neighbour
  4,//5 neighbour
  0,//6 neighbour
};

void setup() {
  // Blank
}


void loop() {

  // count neighbors we have right now
  int numNeighbors = 0;

  FOREACH_FACE(f) {

    if ( !isValueReceivedOnFaceExpired( f ) ) {      // Have we seen an neighbor on this face recently?
      numNeighbors++;
    }
  
  }

  int brightness = brightnessArray[numNeighbors];//255/pow(2,numNeighbors);

  // look up the color to show based on number of neighbors
  // No need to bounds check here since we know we can never see more than 6 neighbors 
  // because we only have 6 sides.
  setColor(makeColorHSB( 0, 255, brightness));
  // setColor( colors[ numNeighbors ] );
  
}


