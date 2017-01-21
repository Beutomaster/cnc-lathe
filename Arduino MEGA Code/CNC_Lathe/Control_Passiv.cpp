#include "Control_Passiv.h"

boolean get_control_active() { //not detected with interrupt, because of switch-bouncing
	boolean control_active = digitalRead(PIN_CONTROL_ACTIVE);
  if (control_active) {
    STATE1 |= _BV(STATE1_CONTROL_ACTIVE_BIT); //set STATE1_bit0 = control_active
    //Observing old Control
    disable_observe_machine_PCINT2();
  }
  else {
    if ((STATE1>>STATE1_CONTROL_ACTIVE_BIT)&1) { //if control was active
      reset_initialization();
    }
    STATE1 &= ~_BV(STATE1_CONTROL_ACTIVE_BIT); //delete STATE1_bit0 = control_active
    //Observing old Control
    enable_observe_machine_PCINT2();
  }
  return control_active;
}

void observe_machine() {
  /*
  //now called by interrupt
  if (get_stepper_on_off()) {
    get_current_x_step();
    get_current_z_step();
  }
  */
  //get feed and direction
  get_feed(); //maybe do not call it every time
  //maybe count from last coordinates
  //maybe get tool position (not wired yet)
}

void enable_observe_machine_PCINT2() {
  PCICR |= _BV(PCIE2);    // Pin Change Interrupt Control Register : PCIE2 => turn on PCINT2_vect for PCINT23:16
  PCMSK2 = _BV(PCINT19) | _BV(PCINT20) | _BV(PCINT21) | _BV(PCINT22) | _BV(PCINT23); //Mask for Pins A11-15
}

void disable_observe_machine_PCINT2() {
  PCICR &= ~_BV(PCIE2);    // Pin Change Interrupt Control Register : PCIE2 => turn off PCINT2_vect for PCINT23:16
}

ISR(PCINT2_vect){
  //Pin Change on, PCINT16 - PCINT23
  //detect stepper off !!! (X-Stepper)
  if (digitalRead(PIN_OLD_CONTROL_STEPPER_X_OFF)){
    STATE1 &= ~(_BV(STATE1_STEPPER_BIT)); //delete STATE1_bit7 = STATE1_STEPPER_BIT (Stepper off)
  }
  else {
    STATE1 |= _BV(STATE1_STEPPER_BIT); //set STATE1_bit7 = STATE1_STEPPER_BIT (Stepper on)
    get_current_x_step();
    get_current_z_step();
  }
}    
