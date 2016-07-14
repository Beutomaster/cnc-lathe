#include "CNC_Lathe.h"
#include "Motion_Control.h"
#include "Spindle_Control.h"
#include "Step_Motor_Control.h"

void set_xz_coordinates() {
}

void get_incremental_coordinates(int abs_X, int abs_Z, int target_X, int target_Z) {
}

void set_xz_move(int inc_X, int inc_Z, int feed, char interpolationmode) {
  int x_feed=feed;
  int z_feed=feed;
 

  if (interpolationmode==0) { //interpolationmode=0 = linear
    //x_feed=inc_X/inc_Z*feed; //not finished
    //z_feed=inc_Z/inc_X*feed; //not finished
  }
  else {                      //interpolationmode=1 = circular
    //???
  }
  
  int x_steps=inc_X;
  int z_steps=inc_Z;
   
  set_x_steps(x_steps, x_feed);
  set_z_steps(z_steps, z_feed);
}

void get_xz_coordinates() {
}

int get_xz_feed() {
	int feed=0; //Stub
	return feed;
}

