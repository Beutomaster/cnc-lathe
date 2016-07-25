#include "Motion_Control.h"

boolean incremental=0;
volatile byte interpolationmode=0;

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

void set_xz_move(int X, int Z, int feed, byte interpolation) {
  //int x_steps=0; //has to be global for ISR
  //int z_steps=0; //has to be global for ISR
  //int x_feed=0; //has to be global for ISR
  //int z_feed=0; //has to be global for ISR
  //int command_time=0;
  command_completed=0;
  STATE_F = feed; 
  interpolationmode=interpolation;

  if (!((STATE>>STATE_STEPPER_BIT)&1)) stepper_on();

  if (incremental){
    X=get_inc_X(X);
    Z=get_inc_Z(Z);
  }

  x_steps = X*STEPS_PER_MM; //not finished, maybe overflow
  z_steps = Z*STEPS_PER_MM; //not finished, maybe overflow

  clk_feed = (long)STATE_F * STEPS_PER_MM; //clk_feed in 1/min (Overflow possible?)

  if (interpolationmode==INTERPOLATION_LINEAR) {
    if (Z==0) {
      x_feed=feed;
    } else x_feed=(long)X*feed/Z;
    if (X==0) {
      z_feed=feed;
    } else z_feed=(long)Z*feed/X;

    clk_xfeed = (long)x_feed * STEPS_PER_MM; //clk_xfeed in 1/min (Overflow possible?)
    clk_zfeed = (long)z_feed * STEPS_PER_MM; //clk_xfeed in 1/min (Overflow possible?)

    //set and start Timer3 for 200Hz
    TCCR3B = 0b00011000; //connect no Input-Compare-PINs, WGM33=1, WGM32=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
    TCCR3A = 0b00000011; //connect no Output-Compare-PINs and WGM31=1, WGM30=1 for Fast PWM
    TCCR3C = 0; //no Force of Output Compare
    OCR3A = 10000; //OCR3A = 16MHz/(Prescaler*F_OCF3A) = 16MHz/(8*200Hz) = 10000
    TCNT3 = 0; //set Start Value
    //Output Compare A Match Interrupt Enable
    TIMSK3 |= _BV(OCIE3A); //set 1
    //Prescaler 8 and Start Timer
    TCCR3B |= _BV(CS31); //set 1

/*
    //calculate time of motion
    if (x_feed==0) {
      command_time=0;
    }
    else command_time=(long)X*100/x_feed;
    
    if (z_feed==0) {
      //command_time=0;
    }
    else if (((long)Z*100/z_feed) > command_time) command_time=(long)Z*100/z_feed;
    command_time += WAIT_TIME; //for savety
*/

  }
  else if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
    //set and start Timer
    
  }
  else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
    //set and start Timer
    
  }

  /*
  set_x_steps(x_steps, x_feed);
  set_z_steps(z_steps, z_feed);

  //set command duration
  command_running(command_time);
  */
}

void get_xz_coordinates() { //calculate Coordinates
  
}

int get_xz_feed() {
	int feed=0; //Stub
	return feed;
}


//should be replaced by command_completed=1; in Stepper and Toolchanger ISR
//still needed for G04

void command_running(int command_time) {
  //Timer for Command has to be set
  
  command_completed=0;
}

void command_completed_ISR() {
  STATE_F = 0;
  command_completed=1;
}

