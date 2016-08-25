#ifndef Motion_Control_h
#define Motion_Control_h

//includes
#include <Arduino.h>
#include "CNC_Lathe.h"
#include "CNC_Control.h"
#include "Spindle_Control.h"
#include "Step_Motor_Control.h"

//defines
#define INTERPOLATION_LINEAR 0
#define INTERPOLATION_CIRCULAR_CLOCKWISE 1
#define INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE 2
#define RAPID_LINEAR_MOVEMENT 3
#define WAIT_TIME 500 //waiting time for savety

extern boolean incremental;
extern volatile byte interpolationmode, i_command_time;
extern volatile long clk_feed, clk_xfeed, clk_zfeed;
extern volatile int command_time;

void set_xz_coordinates(int, int);
int get_inc_X(int abs_X);
int get_inc_Z(int abs_Z);
void set_xz_move(int, int, int, byte);
void get_xz_coordinates();
int get_xz_feed();
void command_running(int);

#endif

