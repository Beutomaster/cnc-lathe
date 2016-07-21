#include "Control_Passiv.h"

boolean get_control_active() {
	boolean control_active = !(digitalRead(PIN_CONTROL_INACTIVE)); //LOW-Active (GND = Control activ)
  //STATE &= ((~(_BV(STATE_CONTROL_ACTIVE_BIT))) | (control_active<<STATE_CONTROL_ACTIVE_BIT)); //set STATE_bit0 = control_active
  if (control_active) {
    STATE |= _BV(STATE_CONTROL_ACTIVE_BIT); //set STATE_bit0 = control_active
  }
  else {
    STATE &= ~(_BV(STATE_CONTROL_ACTIVE_BIT)); //delete STATE_bit0 = control_active
  }
	return control_active;
}

void observe_machine() {
  //functions should be able to observe the machine in control active and passive mode
  get_current_x_step(); //maybe better in an ISR
  get_current_z_step(); //maybe better in an ISR
  get_feed();
  //get feed and direction
  //maybe count from last coordinates
  //maybe get tool position (not wired yet)
}

