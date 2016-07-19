#include "Step_Motor_Control.h"

char current_x_step;
char current_z_step;

Stepper xstepper(XSTEPS_PER_TURN, PIN_STEPPER_X_A, PIN_STEPPER_X_B); //configure X-Motor
Stepper zstepper(ZSTEPS_PER_TURN, PIN_STEPPER_Z_A, PIN_STEPPER_Z_B); //configure Z-Motor

void stepper_on() {
  STATE |= _BV(STATE_STEPPER_BIT); //set STATE_bit6 = STATE_STEPPER_BIT
  //turn stepper on with current_x_step & current_z_step
  //???
}

void stepper_off() {
  STATE &= ~(_BV(STATE_STEPPER_BIT)); //delete STATE_bit6 = STATE_STEPPER_BIT
  //???
}

void stepper_timeout() {
  //set timeout for stepper engines active after last move
}

void set_xstepper(int feed, char negativ_direction) {
  //manual control
  if (!((STATE>>STATE_STEPPER_BIT)&1)) stepper_on();
  //set signal with feed and direction
  //???
  STATE_F = feed;
  //set timeout for movement and reset STATE_F
  command_running(MANUAL_IMPULSE);
}

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

//can not run in both directions
void set_x_steps(int x_steps, int x_feed) {
	int rpms=x_feed; //not finished
	xstepper.setSpeed(rpms);
	xstepper.step(x_steps);
}

void set_z_steps(int z_steps, int z_feed) {
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

void get_current_x_step() { //needed to switch on stepper without movement
	current_x_step=0; //stub
  //???
}

void get_current_z_step() { //needed to switch on stepper without movement, save in eprom
	current_z_step=0; //stub
  //???
}

//Stepper-Timeout-ISR:
void stepper_timeout_ISR() {
  stepper_off();
}
