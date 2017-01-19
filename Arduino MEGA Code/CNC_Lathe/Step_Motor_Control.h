#ifndef Step_Motor_Control_h
#define Step_Motor_Control_h

//includes
#include "CNC_Lathe.h"
#ifdef STEPPER_BIB
  #include <Stepper.h> //we can't use it, because it uses a while loop instead of a timer!!!
#endif

//defines
//Stepper
#define STEPS_PER_MM 72 //Steps/mm
#define XSTEPS_PER_TURN 72 //Steps per x-turn
#define ZSTEPS_PER_TURN 72 //Steps per z-turn
#define MANUAL_IMPULSE 100 //time stepper is active, if key is pressed (not used at the moment)
#define RAPID_MAX 4692 //Timer-Value for Feed=799
#define RAPID_MIN 7514 //Timer-Value for Feed=499
#define STEPPER_TIMEOUT_MS 1200000 //Stepper Timeout in ms (20 min)
#define STEPPER_STEP_T_MIN 1000 //min Time per Step in us (F = 799 mm/min => 1043us)
#define STEPPER_STEP_T_MAX 420000 //max Time per Step in us (F = 2 mm/min => 416666us)

#ifdef STEPPER_BIB
  extern Stepper xstepper; //X-Motor-Object
  extern Stepper zstepper; //Z-Motor-Object
#endif
extern volatile int x_step;
extern volatile int z_step;
extern volatile int x_steps; //has to be global for ISR
extern volatile int z_steps; //has to be global for ISR
extern volatile int x_feed; //has to be global for ISR
extern volatile int z_feed; //has to be global for ISR
extern volatile long clk_feed; //clk_feed in 1/1024s (Overflow possible?)
extern volatile long clk_xfeed, clk_zfeed; //clk_feed in 1/1024s
extern volatile int phi_x;
extern volatile int phi_z;
extern volatile byte current_x_step, current_z_step, last_x_step, last_z_step;
extern volatile boolean reset_stepper_timeout;

void stepper_on();
void stepper_off();
void set_xstep(byte);
void set_zstep(byte);
void stepper_timeout();
void set_xz_stepper_manual(int, char, char);
void set_xz_stepper_manual_direct(int, char, char);
void set_x_steps(int, int);
void set_z_steps(int, int);
void get_current_x_step();
void get_current_z_step();
void get_stepper_on_off();
void get_feed();
void save_current_x_step();
void save_current_z_step();
void save_current_x_coordinate();
void save_current_z_coordinate();
void read_last_x_step();
void read_last_z_step();
void read_last_x_coordinate();
void read_last_z_coordinate();

#endif

