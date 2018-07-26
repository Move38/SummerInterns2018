bool neighborsPresent[6] = {false, false, false, false, false, false};
bool neighborsChanged[6];

#define DURATION_FOR_NEIGHBOR_CHANGE 500

Timer bufferForNeighborChange;

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:

  bool didSomethingChange = false;

  // look at each face
  FOREACH_FACE(f) {

    // get current occupied state
    bool isFaceOccupied = !isValueReceivedOnFaceExpired(f);

    // compare last occupied state with current occupied state
    if ( isFaceOccupied == neighborsPresent[f] ) {
      
      // nothing has changed
      if(bufferForNeighborChange.isExpired()) {
      
        // nothing has changed after our buffer time has passed
        neighborsChanged[f] = false;
      }
    }
    else {
      // something changed
      neighborsChanged[f] = true;
      didSomethingChange = true;
      bufferForNeighborChange.set(DURATION_FOR_NEIGHBOR_CHANGE);
    }
  }

  // only if something changed 
  if (didSomethingChange) {
    
    // update our last received
    FOREACH_FACE(f) {
      neighborsPresent[f] = !isValueReceivedOnFaceExpired(f);
    }

    // display which faces have changed
    FOREACH_FACE(f) {
      if ( neighborsChanged[f] ) {
        setFaceColor(f, RED);
      }
      else {
        setFaceColor(f, dim(BLUE, 127));
      }
    }
  }


}
