#include <Arduino.h>
#include "CNC_Lathe.h"
#include "Initialization.h"

//Freigabe oder Eingabe benoetigt.
//Kommunikation ueber Error-Code und Control-Signal

//vars
volatile char initialized=0; //could be replaced by (STATE>>STATE_INIT_BIT)&1

//functions
void intitialize() { //without sensors useless
}

void initialize_tool_position(char tool) {
  if ((tool>0) && (tool<7)) {
    STATE_T = tool;
    STATE |= _BV(STATE_INIT_BIT); //set STATE_bit1 = STATE_INIT
    initialized=1;
  }
}

void reset_initialization() {
  STATE_T = 0;
  STATE &= ~(_BV(STATE_INIT_BIT)); //delete STATE_bit1 = STATE_INIT
  initialized=0;
}

