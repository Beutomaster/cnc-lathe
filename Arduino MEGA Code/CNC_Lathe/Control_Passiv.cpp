#include "Control_Passiv.h"

boolean control_active = 0;

void get_control_active() { //maybe better with interrupt
	boolean control_active = digitalRead(PIN_CONTROL_ACTIVE);
  if (control_active) {
    STATE |= _BV(STATE_CONTROL_ACTIVE_BIT); //set STATE_bit0 = control_active
    //Observing old Control
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_OFF));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_A));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_B));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_A));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_B));
  }
  else {
    if ((STATE>>STATE_CONTROL_ACTIVE_BIT)&1) { //if control was active
      STATE &= ~(_BV(STATE_CONTROL_ACTIVE_BIT)); //delete STATE_bit0 = control_active
      //reset steps
      x_steps = 0;
      z_steps = 0;
      X0 = STATE_X;
      Z0 = STATE_Z;
    }
    //get initional state
    get_stepper_on_off();
    get_current_x_step(); //not really correct, because it sets a new last_step_time
    get_current_z_step(); //not really correct, because it sets a new last_step_time
    //Observing old Control
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_OFF),get_stepper_on_off,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_A),get_current_x_step,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_B),get_current_x_step,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_A),get_current_z_step,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_B),get_current_z_step,CHANGE);
  }
}

void observe_machine() {
  //get feed and direction
  get_feed();
  //maybe count from last coordinates
  //maybe get tool position (not wired yet)
}

