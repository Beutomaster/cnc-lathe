#include "Control_Passiv.h"

boolean get_control_active() { //not detected with interrupt, because of switch-bouncing
	boolean control_active = digitalRead(PIN_CONTROL_ACTIVE);
  if (control_active) {
    STATE1 |= _BV(STATE1_CONTROL_ACTIVE_BIT); //set STATE1_bit0 = control_active
    //Observing old Control
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_OFF));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_A));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_B));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_A));
    detachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_B));
  }
  else {
    if ((STATE1>>STATE1_CONTROL_ACTIVE_BIT)&1) { //if control was active
      reset_initialization();
    }
    STATE1 &= ~(_BV(STATE1_CONTROL_ACTIVE_BIT)); //delete STATE1_bit0 = control_active
    //Observing old Control
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_OFF),get_stepper_on_off,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_A),get_current_x_step,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_B),get_current_x_step,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_A),get_current_z_step,CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_B),get_current_z_step,CHANGE);
  }
  return control_active;
}

void observe_machine() {
  //get_stepper_on_off(); //(now called by interrupt)
  //get_current_x_step(); //not really correct, because it sets a new last_step_time (now called by interrupt)
  //get_current_z_step(); //not really correct, because it sets a new last_step_time (now called by interrupt)
  //get feed and direction
  get_feed();
  //maybe count from last coordinates
  //maybe get tool position (not wired yet)
}

