#include "Step_Motor_Control.h"

//ISR
volatile unsigned int i_T2ISR=0, ix_next=0, iz_next=0;
volatile int x_step=0;
volatile int z_step=0;
volatile int x_steps=0; //has to be global for ISR
volatile int z_steps=0; //has to be global for ISR
volatile int x_feed=0; //has to be global for ISR
volatile int z_feed=0; //has to be global for ISR
volatile int phi_x=0;
volatile int phi_z=0;

volatile byte current_x_step=0, current_z_step=0;

Stepper xstepper(XSTEPS_PER_TURN, PIN_STEPPER_X_A, PIN_STEPPER_X_B); //configure X-Motor
Stepper zstepper(ZSTEPS_PER_TURN, PIN_STEPPER_Z_A, PIN_STEPPER_Z_B); //configure Z-Motor

void stepper_on() {
  STATE |= _BV(STATE_STEPPER_BIT); //set STATE_bit6 = STATE_STEPPER_BIT
  //turn stepper on with last_x_step & last_z_step (at Init from eeprom)
  set_xstep(current_x_step);
  set_zstep(current_z_step);
}

void stepper_off() {
  //A,B,C,D LOW (We have to change the logic, because Signals at the moment are always C=!A, D=!B !!!)
  digitalWrite(PIN_STEPPER_X_A, LOW);
  digitalWrite(PIN_STEPPER_X_B, LOW);
  digitalWrite(PIN_STEPPER_X_C, LOW);
  digitalWrite(PIN_STEPPER_X_D, LOW);
  digitalWrite(PIN_STEPPER_Z_A, LOW);
  digitalWrite(PIN_STEPPER_Z_B, LOW);
  digitalWrite(PIN_STEPPER_Z_C, LOW);
  digitalWrite(PIN_STEPPER_Z_D, LOW);
  STATE &= ~(_BV(STATE_STEPPER_BIT)); //delete STATE_bit6 = STATE_STEPPER_BIT
}

void set_xstep(byte nextstep) { //right step order for +X has to be verified
  switch (nextstep) {
      case 0:  // 1010
        digitalWrite(PIN_STEPPER_X_A, HIGH);
        digitalWrite(PIN_STEPPER_X_B, LOW);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, LOW);
        current_x_step = 0;
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, LOW);
        current_x_step = 1;
      break;
      case 2:  //0101
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, LOW);
        digitalWrite(PIN_STEPPER_X_D, HIGH);
        current_x_step = 2;
      break;
      case 3:  //1001
        digitalWrite(PIN_STEPPER_X_A, HIGH);
        digitalWrite(PIN_STEPPER_X_B, LOW);
        digitalWrite(PIN_STEPPER_X_C, LOW);
        digitalWrite(PIN_STEPPER_X_D, HIGH);
        current_x_step = 3;
    }
}

void set_zstep(byte nextstep) { //right step order for +Z has to be verified
  switch (nextstep) {
      case 0:  // 1010
        digitalWrite(PIN_STEPPER_Z_A, HIGH);
        digitalWrite(PIN_STEPPER_Z_B, LOW);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
        current_z_step = 0;
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
        current_z_step = 1;
      break;
      case 2:  //0101
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, LOW);
        digitalWrite(PIN_STEPPER_Z_D, HIGH);
        current_z_step = 2;
      break;
      case 3:  //1001
        digitalWrite(PIN_STEPPER_Z_A, HIGH);
        digitalWrite(PIN_STEPPER_Z_B, LOW);
        digitalWrite(PIN_STEPPER_Z_C, LOW);
        digitalWrite(PIN_STEPPER_Z_D, HIGH);
        current_z_step = 3;
    }
}

void stepper_timeout() {
  //set timeout for stepper engines active after last move
}

//continuous movement for manual control (maybe not needed)
void set_xstepper(int feed, char negativ_direction) {
  //manual control
  if (!((STATE>>STATE_STEPPER_BIT)&1)) stepper_on();
  //set signal with feed and direction
  //???
  STATE_F = feed;
  //set timeout for movement and reset STATE_F
  command_running(MANUAL_IMPULSE);
}

//continuous movement for manual control (maybe not needed)
void set_zstepper(int feed, char negativ_direction) {
  int command_time = MANUAL_IMPULSE;
  //manual control
  if (!((STATE>>STATE_STEPPER_BIT)&1)) stepper_on();
  //set signal with feed and direction
  //???
  STATE_F = feed;
  //set timeout for movement and reset STATE_F
  command_running(MANUAL_IMPULSE);
}

void set_x_steps(int x_steps_local, int x_feed_local) { //maybe not needed anymore
  //direction negativ if x_steps negativ
  /* old Stepper.h
	int rpms=x_feed_local; //not finished
	xstepper.setSpeed(rpms);
	xstepper.step(x_steps_local);
  */
  x_steps=x_steps_local;
  x_feed=x_feed_local;

  //???
}

void set_z_steps(int z_steps_local, int z_feed_local) { //maybe not needed anymore
  //direction negativ if z_steps negativ
  /* old Stepper.h
  int rpms=z_feed_local; //not finished
  zstepper.setSpeed(rpms);
  zstepper.step(z_steps);
  */
  z_steps=z_steps_local;
  z_feed=z_feed_local;

  //???
}

int count_x_steps() {
  int x_steps_moved=0; //stub
  //???
	return x_steps_moved;
}

int count_z_steps() {
	int z_steps_moved=0; //stub
  //???
	return z_steps_moved;
}

void get_current_x_step() { //to observe EMCO Control, maybe better in an ISR
  //needs all four stepper pins to detect stepper off !!!
  current_x_step = 0; //not finished
}

void get_current_z_step() { //to observe EMCO Control, maybe better in an ISR
  //needs all four stepper pins to detect stepper off !!!
  current_z_step = 0; //not finished
}

void get_feed() { //to observe EMCO Control
  STATE_F = 0; //not finished
}

// Write/Erase Cycles:10,000 Flash/100,000 EEPROM

void save_current_x_step() { //needed to switch on stepper without movement, save in eeprom !!!
	EEPROM.write(LAST_X_STEP_ADDRESS, current_x_step);
}

void save_current_z_step() { //needed to switch on stepper without movement, save in eeprom !!!
	EEPROM.write(LAST_Z_STEP_ADDRESS, current_z_step);
}

void read_last_x_step() { //needed to switch on stepper without movement
  current_x_step = EEPROM.read(LAST_X_STEP_ADDRESS);
}

void read_last_z_step() { //needed to switch on stepper without movement
  current_z_step = EEPROM.read(LAST_Z_STEP_ADDRESS);
}

//Stepper-Timeout-ISR:
void stepper_timeout_ISR() {
  stepper_off();
}

ISR(TIMER2_OVF_vect) {
  if (interpolationmode==INTERPOLATION_LINEAR) {
    //not finished
  }
  else {
    //Circular Interpolation with different speed settings for x- and z-stepper
    
    //local Vars
    long clk_xfeed, clk_zfeed;
    
    //Steps have to be seperated in max. 90 sections of same moving average feed.
    //For each of x_steps and z_steps an average phi of the section has to be calculated.
    //Maybe an calculation of the next phi with a modified Bresenham-Algorithm could improve it.
    
    //next X-Step
    if (i_T2ISR == ix_next) {
      phi_x = (long)x_step*90/x_steps;
      
      if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
        //calculation of next x-clk (Direction)
        if (z_step < 0) {
          if (x_step < 0) {
          clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
          }
          else {
          clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
          }
        }
        else {
          if (x_step < 0) {
          clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
          }
          else {
          clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
          }
        }
      }
      else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
        //calculation of next z-clk (Direction)
        if (z_step < 0) {
          if (x_step < 0) {
          clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
          }
          else {
          clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
          }
        }
        else {
          if (x_step < 0) {
          clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
          }
          else {
          clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
          }
        }
      }
      
      //next i for switching
      ix_next += CLK_TIMER2 / clk_xfeed;
      
      //next step in direction
      //every step hast to be executed, feed can't be zero
      if (x_step<=x_steps) {
        if (clk_xfeed) { //clock not zero
          if (x_steps<0) {
            if (current_x_step==0){
              current_x_step=3;
            } else current_x_step--;
          }
          else {
            if (current_x_step==3){
              current_x_step=0;
            } else current_x_step++;
          }
          set_xstep(current_x_step);
        }
      }
    }
    
    //next Z-Step
    if (i_T2ISR == iz_next) {
      phi_z = (long)z_step*90/z_steps;
      
      if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
        //calculation of next x-clk (Direction)
        if (z_step < 0) {
          if (x_step < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
        }
        else {
          if (x_step < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
        }
      }
      else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
        //calculation of next z-clk (Direction)
        if (z_step < 0) {
          if (x_step < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
        }
        else {
          if (x_step < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
        }
      
        //next i for switching
        iz_next += CLK_TIMER2 / clk_zfeed;
        
        //next step in direction
        if (z_step<=z_steps) {
          if (clk_zfeed) { //clock not zero
            if (z_steps<0) {
              if (current_z_step==0){
                current_z_step=3;
              } else current_z_step--;
            }
            else {
              if (current_z_step==3){
                current_z_step=0;
              } else current_z_step++;
            }
            set_zstep(current_z_step);
            }
          }
        }
    }
    

  }
  
  if ((x_step>=x_steps) && (z_step>=z_steps)) {
    int i_T2ISR=0;
    int ix_next=0;
    int iz_next=0;
    phi_x=0;
    phi_z=0;
    x_step=0;
    z_step=0;
    x_steps=0;
    z_steps=0;
    STATE_F = 0;
    command_completed=1;
  }
  
  //counter
  i_T2ISR++;
  
  //reset INTR-flag? OVF is resetted automatically
}
