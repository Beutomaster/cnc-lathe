#include "Step_Motor_Control.h"

byte current_x_step; //has to be set by stepper routine
byte current_z_step; //has to be set by stepper routine

Stepper xstepper(XSTEPS_PER_TURN, PIN_STEPPER_X_A, PIN_STEPPER_X_B); //configure X-Motor
Stepper zstepper(ZSTEPS_PER_TURN, PIN_STEPPER_Z_A, PIN_STEPPER_Z_B); //configure Z-Motor

void stepper_on() {
  STATE |= _BV(STATE_STEPPER_BIT); //set STATE_bit6 = STATE_STEPPER_BIT
  //turn stepper on with last_x_step & last_z_step from eeprom
  //???
}

void stepper_off() {
  STATE &= ~(_BV(STATE_STEPPER_BIT)); //delete STATE_bit6 = STATE_STEPPER_BIT
  //A,B,C,D LOW (We have to change the logic, because Signals at the moment are always C=!A, D=!B !!!)
}

/*
void set_xstep(byte nextstep) {
  switch (nextstep) {
      case 0:  // 1010
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
      case 1:  // 0110
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, HIGH);
        digitalWrite(motor_pin_4, LOW);
      break;
      case 2:  //0101
        digitalWrite(motor_pin_1, LOW);
        digitalWrite(motor_pin_2, HIGH);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
      break;
      case 3:  //1001
        digitalWrite(motor_pin_1, HIGH);
        digitalWrite(motor_pin_2, LOW);
        digitalWrite(motor_pin_3, LOW);
        digitalWrite(motor_pin_4, HIGH);
    }
}
*/

void stepper_timeout() {
  //set timeout for stepper engines active after last move
}

//continuous movement for manual control (maybe not needed)
void set_xstepper(int feed, char negativ_direction) {
  //manual control
  if (!((STATE>>STATE_STEPPER_BIT)&1)) stepper_on();
  //set signal with feed and direction
  //???
  STATE_F = feed;
  //set timeout for movement and reset STATE_F
  command_running(MANUAL_IMPULSE);
}

//continuous movement for manual control (maybe not needed)
void set_zstepper(int feed, char negativ_direction) {
  int command_time = MANUAL_IMPULSE;
  //manual control
  if (!((STATE>>STATE_STEPPER_BIT)&1)) stepper_on();
  //set signal with feed and direction
  //???
  STATE_F = feed;
  //set timeout for movement and reset STATE_F
  command_running(MANUAL_IMPULSE);
}

void set_x_steps(int x_steps, int x_feed) {
  //direction negativ if x_steps negativ
	int rpms=x_feed; //not finished
	xstepper.setSpeed(rpms);
	xstepper.step(x_steps);
}

void set_z_steps(int z_steps, int z_feed) {
  //direction negativ if z_steps negativ
  int rpms=z_feed; //not finished
  zstepper.setSpeed(rpms);
  zstepper.step(z_steps);
}

int count_x_steps() {
  int x_steps_moved=0; //stub
  //???
	return x_steps_moved;
}

int count_z_steps() {
	int z_steps_moved=0; //stub
  //???
	return z_steps_moved;
}

void get_current_x_step() { //to observe EMCO Control, maybe better in an ISR
  //needs all four stepper pins to detect stepper off !!!
  current_x_step = 0; //not finished
}

void get_current_z_step() { //to observe EMCO Control, maybe better in an ISR
  //needs all four stepper pins to detect stepper off !!!
  current_z_step = 0; //not finished
}

void get_feed() { //to observe EMCO Control
  STATE_F = 0; //not finished
}

// Write/Erase Cycles:10,000 Flash/100,000 EEPROM

void save_current_x_step() { //needed to switch on stepper without movement, save in eeprom !!!
	EEPROM.write(LAST_X_STEP_ADDRESS, current_x_step);
}

void save_current_z_step() { //needed to switch on stepper without movement, save in eeprom !!!
	EEPROM.write(LAST_Z_STEP_ADDRESS, current_z_step);
}

void read_last_x_step() { //needed to switch on stepper without movement
  current_x_step = EEPROM.read(LAST_X_STEP_ADDRESS);
}

void read_last_z_step() { //needed to switch on stepper without movement
  current_z_step = EEPROM.read(LAST_Z_STEP_ADDRESS);
}

//Stepper-Timeout-ISR:
void stepper_timeout_ISR() {
  stepper_off();
}
