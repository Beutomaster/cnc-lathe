#ifndef Motion_Control_h
#define Motion_Control_h

#define INTERPOLATION_LINEAR 0
#define INTERPOLATION_CIRCULAR 1

void set_xz_coordinates();
void get_incremental_coordinates(int, int, int, int);
void set_xz_move(int, int, int, char);
void get_xz_coordinates();
int get_xz_feed();

#endif

