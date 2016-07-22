#include "Step_Motor_Control.h"

//ISR
volatile int x_steps=0; //has to be global for ISR
volatile int z_steps=0; //has to be global for ISR
volatile unsigned int i=0, ix_next=0, iz_next=0;
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

void set_xstep(byte nextstep) {
  switch (nextstep) {
      case 0:  // 1010
        digitalWrite(PIN_STEPPER_X_A, HIGH);
        digitalWrite(PIN_STEPPER_X_B, LOW);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, LOW);
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, LOW);
      break;
      case 2:  //0101
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, LOW);
        digitalWrite(PIN_STEPPER_X_D, HIGH);
      break;
      case 3:  //1001
        digitalWrite(PIN_STEPPER_X_A, HIGH);
        digitalWrite(PIN_STEPPER_X_B, LOW);
        digitalWrite(PIN_STEPPER_X_C, LOW);
        digitalWrite(PIN_STEPPER_X_D, HIGH);
    }
}

void set_zstep(byte nextstep) {
  switch (nextstep) {
      case 0:  // 1010
        digitalWrite(PIN_STEPPER_Z_A, HIGH);
        digitalWrite(PIN_STEPPER_Z_B, LOW);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
      break;
      case 2:  //0101
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, LOW);
        digitalWrite(PIN_STEPPER_Z_D, HIGH);
      break;
      case 3:  //1001
        digitalWrite(PIN_STEPPER_Z_A, HIGH);
        digitalWrite(PIN_STEPPER_Z_B, LOW);
        digitalWrite(PIN_STEPPER_Z_C, LOW);
        digitalWrite(PIN_STEPPER_Z_D, HIGH);
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

void set_x_steps(int x_steps, int x_feed) {
  //direction negativ if x_steps negativ
	int rpms=x_feed; //not finished
	xstepper.setSpeed(rpms);
	xstepper.step(x_steps);
}

void set_z_steps(int z_steps, int z_feed) {
  //direction negativ if z_steps negativ
  int rpms=z_feed; //not finished
  zstepper.setSpeed(rpms);
  zstepper.step(z_steps);
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
  //maybe seperation in circle-sections needed
  
  //actual x/z-feed
  int x_feed = ((long)STATE_F * lookup_cosinus[i])>>15; //x_steps is missing in this calculation and i is not correct (increments with 62,5 kHz)
  int z_feed = ((long)STATE_F * lookup_cosinus[91-i])>>15; //z_steps is missing in this calculation and i is not correct (increments with 62,5 kHz)

  //next i
  long clk_x =(long)x_feed * STEPS_PER_MM; //clk_x in 1/min
  long clk_z =(long)z_feed * STEPS_PER_MM; //clk_z in 1/min

  ix_next = CLK_TIMER2 / clk_x; //not shure, if this creates a circle

  //X-Steps
  if ((i%ix_next)==0) {
    //next step in direction
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

  //Z-Steps
  if ((i%iz_next)==0) {
    //next step in direction
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

  //counter
  i++;
  
  //reset INTR-flag
}
