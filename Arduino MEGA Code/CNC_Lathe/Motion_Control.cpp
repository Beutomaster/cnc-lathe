#include "Motion_Control.h"

boolean incremental=0;
//ISR
volatile unsigned int i=0, ISR_X=0, ISR_Z=0, ix_next=0, iz_next=0;
volatile byte jxstep=0, jzstep=0;

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

void set_xz_move(int X, int Z, int feed, byte interpolationmode) {
  int x_steps=0;
  int z_steps=0;
  int x_feed=0;
  int z_feed=0;
  int command_time=0;
  STATE_F = feed;

  if (incremental){
    X=get_inc_X(X);
    Z=get_inc_Z(Z);
  }

  ISR_X=X;
  ISR_Z=Z;

  x_steps = X*STEPS_PER_DISTANCE; //not finished, maybe overflow
  z_steps = Z*STEPS_PER_DISTANCE; //not finished, maybe overflow

  if (interpolationmode==INTERPOLATION_LINEAR) { //interpolationmode = linear
    if (Z==0) {
      x_feed=feed;
    } else x_feed=(long)X*feed/Z; //not finished
    if (X==0) {
      z_feed=feed;
    } else z_feed=(long)Z*feed/X; //not finished

    //calculate time of motion
    if (x_feed==0) {
      command_time=0;
    } command_time=((long)X*100/x_feed); //not finished
    
    if (z_feed==0) {
      command_time=0;
    } if ((Z/z_feed*100) > command_time) command_time=((long)Z*100)/z_feed;  //not finished zfeed could be 0
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

int get_xz_feed() {
	int feed=0; //Stub
	return feed;
}

void command_running(int command_time) {
  //Timer for Command has to be set
  
  command_completed=0;
}

void command_completed_ISR() {
  STATE_F = 0;
  command_completed=1;
}

/*
ISR(TIMER2_OVF_vect) {
  //actual x/z-feed
  int x_feed = ((long)STATE_F * phi[i]))>>15;
  int z_feed = ((long)STATE_F * phi[91-i])>>15;

  //next i
  long clk_x =(long)x_feed * STEPS_PER_DISTANCE; //clk_x in 1/min
  long clk_z =(long)z_feed * STEPS_PER_DISTANCE; //clk_z in 1/min

  ix_next = CLK_TIMER2 / clk_x;

  //X-Steps
  if ((i%ix_next)==0) {
    if (jxstep==0) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_X_A, LOW);
      digitalWrite(PIN_STEPPER_X_B, LOW);
    }

    if (jxstep==1) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_X_A, LOW);
      digitalWrite(PIN_STEPPER_X_B, HIGH);
    }

    if (jxstep==2) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_X_A, HIGH);
      digitalWrite(PIN_STEPPER_X_B, HIGH);
    }

    if (jxstep==3) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_X_A, HIGH);
      digitalWrite(PIN_STEPPER_X_B, LOW);
    }
  }

  //Z-Steps
  if ((i%iz_next)==0) {
    if (jzstep==0) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_Z_A, LOW);
      digitalWrite(PIN_STEPPER_Z_B, LOW);
    }

    if (jzstep==1) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_Z_A, LOW);
      digitalWrite(PIN_STEPPER_Z_B, HIGH);
    }

    if (jzstep==2) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_Z_A, HIGH);
      digitalWrite(PIN_STEPPER_Z_B, HIGH);
    }

    if (jzstep==3) { //in witch step are we actual
      digitalWrite(PIN_STEPPER_Z_A, HIGH);
      digitalWrite(PIN_STEPPER_Z_B, LOW);
    }
  }

  //direction
  if (X<0) {
    if (jxstep==0){
      jxstep=3;
    } else jxstep--;
  }
  else {
    if (jxstep==3){
      jxstep=0;
    } else jxstep++;
  }

  if (Z<0) {
    if (jzstep==0){
      jzstep=3;
    } else jzstep--;
  }
  else {
    if (jzstep==3){
      jzstep=0;
    } else jzstep++;
  }
  
  //counter
  i++;
  
  //reset INTR-flag
}
*/
