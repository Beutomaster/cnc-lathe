#include "Step_Motor_Control.h"

//ISR vars get_current_step
volatile unsigned long xstep_time=0, last_xstep_time=0, zstep_time=0, last_zstep_time=0;

//TIMER ISR vars
volatile int x_step=0;
volatile int z_step=0;
volatile int x_steps=0; //has to be global for ISR
volatile int z_steps=0; //has to be global for ISR
volatile int x_feed=0; //has to be global for ISR
volatile int z_feed=0; //has to be global for ISR
volatile long clk_feed = 0; //clk_feed in 1/min (Overflow possible?)
volatile long clk_xfeed=0, clk_zfeed=0;
volatile int phi_x=0;
volatile int phi_z=0;

volatile byte current_x_step=0, current_z_step=0;

Stepper xstepper(XSTEPS_PER_TURN, PIN_STEPPER_X_A, PIN_STEPPER_X_B); //configure X-Stepper
Stepper zstepper(ZSTEPS_PER_TURN, PIN_STEPPER_Z_A, PIN_STEPPER_Z_B); //configure Z-Stepper

void stepper_on() {
  STATE |= _BV(STATE_STEPPER_BIT); //set STATE_bit7 = STATE_STEPPER_BIT
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
  STATE &= ~(_BV(STATE_STEPPER_BIT)); //delete STATE_bit7 = STATE_STEPPER_BIT
}

void set_xstep(byte nextstep) {
  //+X/+Z: B 90° before A,
  //-X/-Z: A 90° before B,
  //C=!A, D=!B
  switch (nextstep) {
      case 0:  // 0011
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, LOW);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, HIGH);
        current_x_step = 0;
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, LOW);
        current_x_step = 1;
      break;
      case 2:  //1100
        digitalWrite(PIN_STEPPER_X_A, HIGH);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, LOW);
        digitalWrite(PIN_STEPPER_X_D, LOW);
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

void set_zstep(byte nextstep) {
  //+X/+Z: B 90° before A,
  //-X/-Z: A 90° before B,
  //C=!A, D=!B
  switch (nextstep) {
      case 0:  // 0011
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, LOW);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, HIGH);
        current_z_step = 0;
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
        current_z_step = 1;
      break;
      case 2:  //1100
        digitalWrite(PIN_STEPPER_Z_A, HIGH);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, LOW);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
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

void get_current_x_step() { //to observe EMCO Control (ISR)
  //needs all four stepper pins to detect stepper off !!!
  //Problem: Switching Stepper off in Step 0 can't be detected
  last_xstep_time=xstep_time;
  xstep_time=micros();
  byte step_bincode;
  step_bincode = ((byte)(digitalRead(PIN_OLD_CONTROL_STEPPER_X_A))<<1);
  step_bincode |= (byte)(digitalRead(PIN_OLD_CONTROL_STEPPER_X_B));
  switch (step_bincode) {
    case 0: current_x_step = 0;
            break;
    case 1: current_x_step = 1;
            break;
    case 3: current_x_step = 2;
            break;
    case 2: current_x_step = 3;
  }
}

void get_current_z_step() { //to observe EMCO Control (ISR)
  //needs all four stepper pins to detect stepper off !!!
  //Problem: Switching Stepper off in Step 0 can't be detected
  last_zstep_time=zstep_time;
  zstep_time=micros();
  byte step_bincode;
  step_bincode = ((byte)(digitalRead(PIN_OLD_CONTROL_STEPPER_Z_A))<<1);
  step_bincode |= (byte)(digitalRead(PIN_OLD_CONTROL_STEPPER_Z_B));
  switch (step_bincode) {
    case 0: current_z_step = 0;
            break;
    case 1: current_z_step = 1;
            break;
    case 3: current_z_step = 2;
            break;
    case 2: current_z_step = 3;
  }
}

void get_stepper_on_off() { //to observe EMCO Control (ISR)
  //detect stepper off !!! (X-Stepper)
  if (digitalRead(PIN_OLD_CONTROL_STEPPER_X_OFF)){
    STATE &= ~(_BV(STATE_STEPPER_BIT)); //delete STATE_bit7 = STATE_STEPPER_BIT (Stepper off)
  }
  else {
    STATE |= _BV(STATE_STEPPER_BIT); //set STATE_bit7 = STATE_STEPPER_BIT (Stepper off)
  }
}

void get_feed() { //to observe EMCO Control
  //Errors at overflow of TIMER0
  long x_feed = 60000000L/((xstep_time-last_xstep_time)*STEPS_PER_MM); //(60s/min)*(1000ms/s)*(1000us/ms) = 60000000
  long z_feed = 60000000L/((zstep_time-last_zstep_time)*STEPS_PER_MM); //(60s/min)*(1000ms/s)*(1000us/ms) = 60000000
  STATE_F = sqrt(x_feed*x_feed+z_feed*z_feed);
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

ISR(TIMER1_OVF_vect) {
  if (command_time) { //Dwell
    if (i_command_time==1) {
      ICR1 = (15625L*command_time/100)-1; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*command_time/(1024*100))-1 = (15625Hz*command_time/100)-1
      if (ICR1>TCNT1) {
        TCNT1=0; //Checks if Timer already overrun the compare value
        //set Interrupt flag???
      }
    }
    else if (!i_command_time) {
        //If time is over
        TCCR1B = 0; //Disable Timer
        command_completed=1;
        command_time=0;
    }
    i_command_time--;
  }

  else {  //X-Stepper
    //next step in direction
    //Movement in -X-Direction
    if (x_steps < 0) {
      if (current_x_step==0) {
        current_x_step=3;
      }
      else current_x_step--;
      x_step--;
      STATE_X--; //not finished, correction needed
    }
    //Movement in +X-Direction
    else {
      if (current_x_step==3) {
        current_x_step=0;
      }
      else current_x_step++;
      x_step++;
      STATE_X++; //not finished, correction needed
    }
    
    //set next step
    set_xstep(current_x_step);

    if (x_step==x_steps) { //last step reached?
      phi_x=0;
      x_step=0;
      x_steps=0;
      x_command_completed=1;
      TCCR1B = 0; //Disable Timer
    }
    else { //next Timer-Compare-Value
      if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE || interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
        //Circular Interpolation with different speed settings for x- and z-stepper
        //Steps have to be seperated in max. 90 sections of same moving average feed.
        //For each of x_steps and z_steps an average phi of the section has to be calculated.
        //Maybe an calculation of the next phi with a modified Bresenham-Algorithm could improve it.
        
        //next X-Step moving average feed
        phi_x = (((long)(x_step))*90+45)/x_steps;
        
        if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
          //calculation of next x-clk (Direction)
          if (z_steps < 0) {
            if (x_steps < 0) {
            clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
            }
            else {
            clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
            }
          }
          else {
            if (x_steps < 0) {
            clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
            }
            else {
            clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
            }
          }
        }
        else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
          //calculation of next x-clk (Direction)
          if (z_steps < 0) {
            if (x_steps < 0) {
            clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
            }
            else {
            clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
            }
          }
          else {
            if (x_steps < 0) {
            clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
            }
            else {
            clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
            }
          }
        }
        
        //next Timer-Compare-Value
        //every step hast to be executed, feed can't be zero
        if (clk_xfeed) { //clock not zero
          ICR1 = (15625L/clk_xfeed)-1; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz/(1024*clk_xfeed))-1 = (15625Hz/clk_xfeed)-1
        } else ICR1 = 15624L;
      }
      
      else if (interpolationmode==RAPID_LINEAR_MOVEMENT) {
      //set Timer-Compare-Values
        if (x_steps) {
          if (x_step < x_steps/2) {
            if (ICR1>RAPID_MAX) {
              ICR1 = RAPID_MIN-x_step*10; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz/(1024*clk_xfeed))-1 = (15625Hz*60/499s)-1
              if (ICR1<RAPID_MAX) {
                ICR1=RAPID_MAX;
              }
            }
          } else if ((x_steps-x_step) < 17) {
            ICR1 = RAPID_MIN-(x_steps-x_step)*10; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz/(1024*clk_xfeed))-1 = (15625Hz*60/499s)-1
              if (ICR1>RAPID_MIN) {
                ICR1=RAPID_MIN;
              }
          }
        }
      }
    }
  }
}

ISR(TIMER3_OVF_vect) {   //Z-Stepper
  //many things to do
  //a different timer operation mode or interrupt is needed for suitable frequency
  //many calculations could be done before starting the timer
  //changing timer settings inside the ISR could replace some calculations and optimize CPU-time
  //Start-/Stop-Frequency

  //next step in direction
  //Movement in -Z-Direction
  if (z_steps < 0) {
    if (current_z_step==0) {
      current_z_step=3;
    }
    else current_z_step--;
    z_step--;
    STATE_Z--; //not finished, correction needed
  }
  //Movement in +Z-Direction
  else {
    if (current_z_step==3) {
      current_z_step=0;
    }
    else current_z_step++;
    z_step++;
    STATE_Z++; //not finished, correction needed
  }
  
  //set next step
  set_zstep(current_z_step);

  if (z_step==z_steps) { //last step reached?
    phi_z=0;
    z_step=0;
    z_steps=0;
    z_command_completed=1;
    TCCR3B = 0; //Disable Timer
  }
  else { //next Timer-Compare-Value
    if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE || interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
      //Circular Interpolation with different speed settings for x- and z-stepper
      //Steps have to be seperated in max. 90 sections of same moving average feed.
      //For each of x_steps and z_steps an average phi of the section has to be calculated.
      //Maybe an calculation of the next phi with a modified Bresenham-Algorithm could improve it.
      
      //next Z-Step moving average feed
      phi_z = (((long)(z_step))*90+45)/z_steps;
      
      if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
        //calculation of next z-clk (Direction)
        if (z_steps < 0) {
          if (x_steps < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
        }
        else {
          if (x_steps < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
        }
      }
      else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
        //calculation of next z-clk (Direction)
        if (z_steps < 0) {
          if (x_steps < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
        }
        else {
          if (x_steps < 0) {
          clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
          }
          else {
          clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
          }
        }
      }
      
      //next Timer-Compare-Value
      //every step hast to be executed, feed can't be zero
      if (clk_zfeed) { //clock not zero
        ICR3 = (15625L/clk_zfeed)-1; //ICR3 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz/(1024*clk_zfeed))-1 = (15625Hz/clk_zfeed)-1
      } else ICR3 = 15624L;
    }

    else if (interpolationmode==RAPID_LINEAR_MOVEMENT) {
        //set Timer-Compare-Values
          if (z_steps) {
            if (z_step < z_steps/2) {
              if (ICR1>RAPID_MAX) {
                ICR3 = RAPID_MIN-z_step*10; //ICR1 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz/(1024*clk_zfeed))-1 = (15625Hz*60/499s)-1
                if (ICR3<RAPID_MAX) {
                  ICR3=RAPID_MAX;
                }
              }
            } else if ((z_steps-z_step) < 17) {
              ICR3 = RAPID_MIN-(z_steps-z_step)*10; //ICR1 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz/(1024*clk_zfeed))-1 = (15625Hz*60/499s)-1
                if (ICR3>RAPID_MIN) {
                  ICR3=RAPID_MIN;
                }
            }
          }
        }
  }
  //reset INTR-flag? OVF resets automatically
}

