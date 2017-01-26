#ifndef Motion_Control_h
#define Motion_Control_h

//includes
#include "CNC_Lathe.h"

//defines
#define INTERPOLATION_LINEAR 0
#define INTERPOLATION_CIRCULAR_CLOCKWISE 1
#define INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE 2
#define RAPID_LINEAR_MOVEMENT 3
#define FEED_IN_MM_PER_MIN 0
#define FEED_IN_MM_PER_REVOLUTION 1
#define FEED_IN_M_PER_MIN_AT_INCR_REVOLUTIONS 2

extern boolean absolute, feed_modus;
extern volatile byte interpolationmode, i_command_time;
extern volatile long clk_feed, clk_xfeed, clk_zfeed;
extern volatile int command_time;
extern volatile int X0, Z0;

void set_xz_coordinates(int, int);
void set_x_coordinate(int);
void set_z_coordinate(int);
int get_inc_X(int);
int get_inc_Z(int);
int get_Tool_X(int);
int get_Tool_Z(int);
void set_xz_move(int, int, int, byte);
int get_xz_coordinates(int, int);
int get_xz_feed();
int get_xz_feed_related_to_revolutions(int);
void command_running(int);

#endif

