#include "Control_Passiv.h"

boolean get_control_active() {
	boolean control_active = digitalRead(PIN_CONTROL_ACTIVE);
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
  //get feed and direction
  get_feed();
  //maybe count from last coordinates
  //maybe get tool position (not wired yet)
}

