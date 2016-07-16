#ifndef Motion_Control_h
#define Motion_Control_h

#define INTERPOLATION_LINEAR 0
#define INTERPOLATION_CIRCULAR_CLOCKWISE 1
#define INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE 2
#define WAIT_TIME 1000; //waiting time for savety
#define STEPS_PER_DISTANCE 1 //needs the right value in the right Q-Format

extern char incremental;

void set_xz_coordinates(int, int);
int get_inc_X(int abs_X);
int get_inc_Z(int abs_Z);
void set_xz_move(int, int, int, char);
void get_xz_coordinates();
int get_xz_feed();
void command_running(int);

#endif

