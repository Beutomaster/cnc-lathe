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
#define WAIT_TIME 1000; //waiting time for savety
#define STEPS_PER_HUNDRETS_OF_MM 23592 // 9Steps/(125*10^-6 m)*2^15 in Q15
#define STEPS_PER_DISTANCE 18432 // 9Steps/(125*10^-3 m)*2^8 in Q8
#define CLK_TIMER2 3750000 //in 1/min, CLK_T2=CLK_IO/(Prescaler 256)

//Cosinus LookUp-Table for Quarter Circle in Q15
//volatile int lookup_cosinus[91] = {32768, 32763, 32748, 32723, 32688, 32643, 32588, 32524, 32449, 32365, 32270, 32166, 32052, 31928, 31795, 31651, 31499, 31336, 31164, 30983, 30792, 30592, 30382, 30163, 29935, 29698, 29452, 29197, 28932, 28660, 28378, 28088, 27789, 27482, 27166, 26842, 26510, 26170, 25822, 25466, 25102, 24730, 24351, 23965, 23571, 23170, 22763, 22348, 21926, 21498, 21063, 20622, 20174, 19720, 19261, 18795, 18324, 17847, 17364, 16877, 16384, 15886, 15384, 14876, 14365, 13848, 13328, 12803, 12275, 11743, 11207, 10668, 10126, 9580, 9032, 8481, 7927, 7371, 6813, 6252, 5690, 5126, 4560, 3993, 3425, 2856, 2286, 1715, 1144, 572, 0};

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

