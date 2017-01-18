#include "Initialization.h"

//Freigabe oder Eingabe benoetigt.
//Kommunikation ueber Error-Code und Control-Signal

//vars
volatile boolean initialized=0; //could be replaced by (STATE1>>STATE1_INIT_BIT)&1

/* obsolete
//functions
void intitialize() { //without sensors useless
}

void initialize_tool_position(byte tool) {
  if ((tool>0) && (tool<7)) {
    STATE_T = tool;
    STATE1 |= _BV(STATE1_INIT_BIT); //set STATE1_bit1 = STATE_INIT
    initialized=1;
  }
}
*/

void reset_initialization() { 
  command_time = 0;
  i_command_time = 0;
  spindle_direction(false); //includes spindle_off()
  stepper_off();
  
  //Tool Changer
  digitalWrite(PIN_TOOL_CHANGER_CHANGE, LOW);
  digitalWrite(PIN_TOOL_CHANGER_HOLD, HIGH);
  //Input Compare Match Interrupt Disable
  TIMSK1 &= ~(_BV(ICIE1)); //set 0

  //Stepper
  

  //reset steps
  x_steps = 0;
  z_steps = 0;
  X0 = STATE_X;
  Z0 = STATE_Z;
  
  //STATE_X = 0; //reset X-position
  //STATE_Z = 0; //reset Z-position
  //STATE_T = 0; //reset tool-position
  STATE_F = 0; //reset feed
  STATE_H = 0; //reset H
  STATE_N = 0; //reset N

  STATE1 &= ~(_BV(STATE1_INIT_BIT)); //delete STATE1_bit1 = STATE_INIT
  initialized=0;
  programm_stop();
  STATE1 &= ~(_BV(STATE1_INCH_BIT)); //set STATE1_bit4 = 0
  //command_completed = 1;
  
  //Anymore vars needed to be resetted?
}

