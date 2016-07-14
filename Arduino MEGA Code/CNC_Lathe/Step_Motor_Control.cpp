#include "CNC_Lathe.h"
#include "Step_Motor_Control.h"

Stepper xstepper(xsteps_per_turn, PIN_STEPPER_X_A, PIN_STEPPER_X_B); //configure X-Motor
Stepper zstepper(zsteps_per_turn, PIN_STEPPER_Z_A, PIN_STEPPER_Z_B); //configure Z-Motor

void x_stepper(char x_stepper_on, char current_x_step) {
}

void z_stepper(char x_stepper_on, char current_z_step) {
}

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
	return x_steps_moved;
}

int count_z_steps() {
	int z_steps_moved=0; //stub
	return z_steps_moved;
}

char get_current_x_step() {
	int current_x_step=0; //stub
	return current_x_step;
}

char get_current_z_step() {
	int current_z_step=0; //stub
	return current_z_step;
}

