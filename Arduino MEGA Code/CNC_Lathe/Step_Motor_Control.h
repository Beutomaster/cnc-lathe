#ifndef Step_Motor_Control_h
#define Step_Motor_Control_h

//includes
#include <Arduino.h>
#include "CNC_Lathe.h"
#include <Stepper.h> //we can't use it, because it uses a while loop instead of a timer!!!

//defines
//Stepper
#define STEPS_PER_MM 72 //Steps/mm
#define XSTEPS_PER_TURN 72 //Steps per x-turn
#define ZSTEPS_PER_TURN 72 //Steps per z-turn
#define MANUAL_IMPULSE 10 //time stepper is active, if key is pressed

extern Stepper xstepper; //X-Motor-Object
extern Stepper zstepper; //Z-Motor-Object
extern volatile unsigned int i_T3ISR, ix_next, iz_next;
extern volatile int x_step;
extern volatile int z_step;
extern volatile int x_steps; //has to be global for ISR
extern volatile int z_steps; //has to be global for ISR
extern volatile int x_feed; //has to be global for ISR
extern volatile int z_feed; //has to be global for ISR
extern volatile int phi_x;
extern volatile int phi_z;
extern volatile byte current_x_step, current_z_step;

void stepper_on();
void stepper_off();
void set_xstep(byte);
void set_zstep(byte);
void stepper_timeout();
void set_xstepper(int, char);
void set_zstepper(int, char);
void set_x_steps(int, int);
void set_z_steps(int, int);
int count_x_steps();
int count_z_steps();
void get_current_x_step();
void get_current_z_step();
void get_feed();
void save_current_x_step();
void save_current_z_step();
void read_last_x_step();
void read_last_z_step();
void stepper_timeout_ISR();

#endif

