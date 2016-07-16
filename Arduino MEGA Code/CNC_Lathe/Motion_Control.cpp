#include "CNC_Lathe.h"
#include "CNC_Control.h"
#include "Motion_Control.h"
#include "Spindle_Control.h"
#include "Step_Motor_Control.h"

char incremental=0;

void set_xz_coordinates(int x_origin, int z_origin) {
  STATE_X -= x_origin;
  STATE_Z -= z_origin;
}

int get_inc_X(int abs_X) { //get incremental x-Coordinate
  return abs_X-STATE_X;
}

int get_inc_Z(int abs_Z) { //get incremental z-Coordinate
  return abs_Z-STATE_Z;
}

void set_xz_move(int X, int Z, int feed, char interpolationmode) {
  int x_steps=0;
  int z_steps=0;
  int x_feed=0;
  int z_feed=0;
  int command_time=0;

  if (incremental){
    X=get_inc_X(X);
    Z=get_inc_Z(Z);
  }

  x_steps = (X*STEPS_PER_DISTANCE)>>0; //not finished, needs right fixpoint values
  z_steps = (Z*STEPS_PER_DISTANCE)>>0; //not finished, needs right fixpoint values

  if (interpolationmode==INTERPOLATION_LINEAR) { //interpolationmode = linear
    x_feed=X/Z*feed; //not finished
    z_feed=Z/X*feed; //not finished

    //calculate time of motion
    command_time=X/x_feed;
    if ((Z/z_feed) > command_time) command_time=Z/z_feed;
    command_time += WAIT_TIME; //for savety
    
  }
  else if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE)
  {                      //interpolationmode = CIRCULAR_CLOCKWISE
    //???
  }
  else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE)
  {                      //interpolationmode = CIRCULAR_COUNTERCLOCKWISE
    //???
  }
    
  set_x_steps(x_steps, x_feed);
  set_z_steps(z_steps, z_feed);

  //set command duration
  command_running(command_time);
}

void get_xz_coordinates() { //calculate Coordinates
  
}

int get_xz_feed() { //readout rpm-sensor
	int feed=0; //Stub
	return feed;
}

void command_running(int command_time) {
  //Timer for Command has to be set
  
  command_completed=0;
}

