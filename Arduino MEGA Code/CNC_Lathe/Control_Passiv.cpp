#include "CNC_Lathe.h"
#include "Control_Passiv.h"
#include <Arduino.h>

char get_control_active() {
	char control_active = !(digitalRead(PIN_CONTROL_INACTIVE)); //LOW-Active (GND = Control activ)
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
}

