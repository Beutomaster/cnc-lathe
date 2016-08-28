#include "Initialization.h"

//Freigabe oder Eingabe benoetigt.
//Kommunikation ueber Error-Code und Control-Signal

//vars
volatile boolean initialized=0; //could be replaced by (STATE>>STATE_INIT_BIT)&1

/* obsolete
//functions
void intitialize() { //without sensors useless
}

void initialize_tool_position(byte tool) {
  if ((tool>0) && (tool<7)) {
    STATE_T = tool;
    STATE |= _BV(STATE_INIT_BIT); //set STATE_bit1 = STATE_INIT
    initialized=1;
  }
}
*/

void reset_initialization() {
  STATE_X = 0; //reset X-position
  STATE_Z = 0; //reset Z-position
  STATE_F = 0; //reset feed
  STATE_H = 0; //reset H
  STATE_T = 0; //reset tool-position
  STATE_H = 0; //reset N
  spindle_off();
  stepper_off();
  STATE &= ~(_BV(STATE_INIT_BIT)); //delete STATE_bit1 = STATE_INIT
  initialized=0;
  programm_stop();
  STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
  //Anymore vars needed to be resetted?
}

