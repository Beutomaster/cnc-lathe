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
  spindle_direction(false); //includes spindle_off()
  command_time = 0;
  i_command_time = 0;
  stepper_off();
  
  //Tool Changer
  digitalWrite(PIN_TOOL_CHANGER_CHANGE, LOW);
  digitalWrite(PIN_TOOL_CHANGER_HOLD, HIGH);
  //Input Compare Match Interrupt Disable
  TIMSK1 &= ~(_BV(ICIE1)); //set 0
  
  //STATE_X = 0; //reset X-position
  //STATE_Z = 0; //reset Z-position
  //STATE_T = 0; //reset tool-position
  STATE_F = 0; //reset feed
  STATE_H = 0; //reset H
  STATE_N = 0; //reset N

  STATE &= ~(_BV(STATE_INIT_BIT)); //delete STATE_bit1 = STATE_INIT
  initialized=0;
  programm_stop();
  STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
  command_completed = 1;
  
  //Anymore vars needed to be resetted?
}

