#ifndef Motion_Control_h
#define Motion_Control_h

//Defined LookUp for Quarter Circle
//volatile int lookup_cosinus[91] = {10000, 9998, 9993, 9986, 9976, 9962, 9945, 9925, 9903, 9877, 9848, 9816, 9781, 9743, 9702, 9659, 

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
#define WAIT_TIME 1000; //waiting time for savety
#define STEPS_PER_DISTANCE 1 //needs the right value in the right Q-Format

extern boolean incremental;

void set_xz_coordinates(int, int);
int get_inc_X(int abs_X);
int get_inc_Z(int abs_Z);
void set_xz_move(int, int, int, char);
void get_xz_coordinates();
int get_xz_feed();
void command_running(int);
void command_completed_ISR();

#endif

