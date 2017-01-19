#include "Step_Motor_Control.h"

//ISR vars get_current_step
volatile unsigned long xstep_time=0, last_xstep_time=0, zstep_time=0, last_zstep_time=0, stepper_timeout_timestamp=0;

//TIMER ISR vars
volatile int x_step=0;
volatile int z_step=0;
volatile int x_steps=0; //has to be global for ISR
volatile int z_steps=0; //has to be global for ISR
volatile int x_feed=0; //has to be global for ISR
volatile int z_feed=0; //has to be global for ISR
volatile long clk_feed = 0; //clk_feed in 1/min
volatile long clk_xfeed=0, clk_zfeed=0;
volatile int phi_x=0;
volatile int phi_z=0;

volatile byte current_x_step=0, current_z_step=0, last_x_step=0, last_z_step=0;
volatile boolean reset_stepper_timeout=false;

#ifdef STEPPER_BIB
  Stepper xstepper(XSTEPS_PER_TURN, PIN_STEPPER_X_A, PIN_STEPPER_X_B); //configure X-Stepper
  Stepper zstepper(ZSTEPS_PER_TURN, PIN_STEPPER_Z_A, PIN_STEPPER_Z_B); //configure Z-Stepper
#endif

void stepper_on() {
  STATE1 |= _BV(STATE1_STEPPER_BIT); //set STATE1_bit7 = STATE1_STEPPER_BIT
  //turn stepper on with last_x_step & last_z_step (at Init from eeprom)
  set_xstep(current_x_step);
  set_zstep(current_z_step);
}

void stepper_off() {
  if (!i_command_time && !command_time) {
    TCCR1B = 0; //Disable Timer 1
    TIMSK1 &= ~(_BV(TOIE1)); //set 0 => Disable OVF1 Interrupt Enable
  }
  TCCR3B = 0; //Disable Timer 3
  TIMSK3 &= ~(_BV(TOIE3)); //set 0 => Disable OVF3 Interrupt Enable
  phi_z=0;
  phi_x=0;
  x_step=0;
  z_step=0;
  x_steps=0;
  z_steps=0;
  x_command_completed=1;
  z_command_completed=1;
  
  //A,B,C,D LOW (We have to change the logic, because Signals at the moment are always C=!A, D=!B !!!)
  digitalWrite(PIN_STEPPER_X_A, LOW);
  digitalWrite(PIN_STEPPER_X_B, LOW);
  digitalWrite(PIN_STEPPER_X_C, LOW);
  digitalWrite(PIN_STEPPER_X_D, LOW);
  digitalWrite(PIN_STEPPER_Z_A, LOW);
  digitalWrite(PIN_STEPPER_Z_B, LOW);
  digitalWrite(PIN_STEPPER_Z_C, LOW);
  digitalWrite(PIN_STEPPER_Z_D, LOW);
  STATE2 &= ~(_BV(STATE2_XSTEPPER_RUNNING_BIT)) & ~(_BV(STATE2_ZSTEPPER_RUNNING_BIT));
  STATE1 &= ~(_BV(STATE1_STEPPER_BIT)); //delete STATE1_bit7 = STATE1_STEPPER_BIT
}

void set_xstep(byte nextstep) {
  //+X/+Z: B 90° before A,
  //-X/-Z: A 90° before B,
  //C=!A, D=!B
  last_x_step = current_x_step;
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
  last_z_step = current_z_step;
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
  if (reset_stepper_timeout) stepper_timeout_timestamp = millis();
  reset_stepper_timeout=false;
  if ((millis() - stepper_timeout_timestamp) > STEPPER_TIMEOUT_MS) stepper_off();
}

//continuous movement for manual control
void set_xz_stepper_manual(int feed, char negativ_direction, char xz_stepper) { //x: xz_stepper=0, z: xz_stepper=1
  //manual control
  //if (!((STATE1>>STATE1_STEPPER_BIT)&1)) stepper_on();
  int X=0, Z=0;
  
  //set signal with feed and direction
  if (command_completed) {
    //calculate needed Coordinates for s/2
    if (xz_stepper) {
      Z = feed * 100 / 120; //100 for mm * min/60s * 1/2
      if (negativ_direction) Z *= -1;
      if (absolute) Z += STATE_Z;
    }
    else {
      X = feed * 100 / 120; //100 for mm * min/60s * 1/2
      if (negativ_direction) X *= -1;
      if (absolute) X += STATE_X;
    }
    set_xz_move(X, Z, feed, INTERPOLATION_LINEAR);
  }
  else {
    //increase steps
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      if (!x_command_completed) {
        x_steps += x_step;
      }
      if (!z_command_completed) {
        z_steps += z_step;
      } 
    }
  }
  
  //set timeout for movement and reset STATE_F
  //command_running(MANUAL_IMPULSE);
}

//continuous movement for manual control second try ... not finished !!!
void set_xz_stepper_manual_direct(int feed, char negativ_direction, char xz_stepper) { //x: xz_stepper=0, z: xz_stepper=1
  command_completed=0;
  
  STATE_F = feed;
  
  interpolationmode=INTERPOLATION_LINEAR;

  //turn stepper on with last step
  if (!((STATE1>>STATE1_STEPPER_BIT)&1)) stepper_on();

  X0 = STATE_X;
  Z0 = STATE_Z;
  x_step=0;
  z_step=0;

  //calculate needed steps for s/2
  if (xz_stepper) { //z_stepper
    x_steps = 0;
    z_steps = feed * STEPS_PER_MM / 120; //min/60s * 1/2
    if (negativ_direction) z_steps *= -1;
    x_command_completed = 0;
    STATE2 |= _BV(STATE2_ZSTEPPER_RUNNING_BIT);
  }
  else { //x_stepper
    z_steps = 0;
    x_steps = feed * STEPS_PER_MM / 120; //min/60s * 1/2
    if (negativ_direction) x_steps *= -1;
    z_command_completed = 0;
    STATE2 |= _BV(STATE2_XSTEPPER_RUNNING_BIT);
  }
  //set signal with feed and direction
  //configure and start Timer
  //... not finished
  
  //set timeout for movement and reset STATE_F
  //command_running(MANUAL_IMPULSE);
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

void get_current_x_step() { //to observe EMCO Control (ISR)
  //needs all four stepper pins to detect stepper off !!!
  //Problem: Switching Stepper off in Step 0 can't be detected
  last_xstep_time=xstep_time;
  xstep_time=micros();
  byte step_bincode;
  step_bincode = ((byte)(digitalRead(PIN_OLD_CONTROL_STEPPER_X_A))<<1);
  step_bincode |= (byte)(digitalRead(PIN_OLD_CONTROL_STEPPER_X_B));
  switch (step_bincode) {
    case 0: //A=B=0
            if (last_x_step == 3) x_steps++;
            if (last_x_step == 1) x_steps--;
            current_x_step = 0;
            break;
    case 1: //A=0,B=1
            if (last_x_step == 0) x_steps++;
            if (last_x_step == 2) x_steps--;
            current_x_step = 1;
            break;
    case 3: //A=1,B=1
            if (last_x_step == 1) x_steps++;
            if (last_x_step == 3) x_steps--;
            current_x_step = 2;
            break;
    case 2: //A=1,B=0
            if (last_x_step == 2) x_steps++;
            if (last_x_step == 0) x_steps--;
            current_x_step = 3;
  }
  last_x_step = current_x_step;
  get_xz_coordinates(X0, x_steps);
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
    case 0: //A=B=0
            if (last_z_step == 3) z_steps++;
            if (last_z_step == 1) z_steps--;
            current_z_step = 0;
            break;
    case 1: //A=0,B=1
            if (last_z_step == 0) z_steps++;
            if (last_z_step == 2) z_steps--;
            current_z_step = 1;
            break;
    case 3: //A=1,B=1
            if (last_z_step == 1) z_steps++;
            if (last_z_step == 3) z_steps--;
            current_z_step = 2;
            break;
    case 2: //A=1,B=0
            if (last_z_step == 2) z_steps++;
            if (last_z_step == 0) z_steps--;
            current_z_step = 3;
  }
  last_z_step = current_z_step;
  get_xz_coordinates(Z0, z_steps);
}

void get_stepper_on_off() { //to observe EMCO Control (ISR)
  //detect stepper off !!! (X-Stepper)
  if (digitalRead(PIN_OLD_CONTROL_STEPPER_X_OFF)){
    STATE1 &= ~(_BV(STATE1_STEPPER_BIT)); //delete STATE1_bit7 = STATE1_STEPPER_BIT (Stepper off)
  }
  else {
    STATE1 |= _BV(STATE1_STEPPER_BIT); //set STATE1_bit7 = STATE1_STEPPER_BIT (Stepper on)
  }
}

void get_feed() { //to observe EMCO Control
  //Errors at overflow of TIMER0
  long x_feed, z_feed; //not really correct!!!
  if (xstep_time-last_xstep_time>0 && xstep_time-last_xstep_time<STEPPER_STEP_T_MAX) x_feed = 60000000L/((long)(xstep_time-last_xstep_time)*STEPS_PER_MM); //(60s/min)*(1000ms/s)*(1000us/ms) = 60000000
  else x_feed=0;
  if (zstep_time-last_zstep_time>0 && zstep_time-last_zstep_time<STEPPER_STEP_T_MAX) z_feed = 60000000L/((long)(zstep_time-last_zstep_time)*STEPS_PER_MM); //(60s/min)*(1000ms/s)*(1000us/ms) = 60000000
  else z_feed=0;
  if (x_feed || z_feed) STATE_F = sqrt((long)x_feed*x_feed+(long)z_feed*z_feed);
  else STATE_F=0;
}

// Write/Erase Cycles:10,000 Flash/100,000 EEPROM

void save_current_x_step() { //needed to switch on stepper without movement, save in eeprom !!!
	EEPROM.update(LAST_X_STEP_ADDRESS, current_x_step);
}

void save_current_z_step() { //needed to switch on stepper without movement, save in eeprom !!!
	EEPROM.update(LAST_Z_STEP_ADDRESS, current_z_step);
}

void save_current_x_coordinate() { //save in eeprom !!!
  EEPROM.update(LAST_X_ADDRESS, STATE_X>>8);
  EEPROM.update(LAST_X_ADDRESS+1, STATE_X);
}

void save_current_z_coordinate() { //save in eeprom !!!
  EEPROM.update(LAST_Z_ADDRESS, STATE_Z>>8);
  EEPROM.update(LAST_Z_ADDRESS+1, STATE_Z);
}

void read_last_x_step() { //needed to switch on stepper without movement
  current_x_step = EEPROM.read(LAST_X_STEP_ADDRESS);
}

void read_last_z_step() { //needed to switch on stepper without movement
  current_z_step = EEPROM.read(LAST_Z_STEP_ADDRESS);
}

void read_last_x_coordinate() {
  STATE_X = ((EEPROM.read(LAST_X_ADDRESS))<<8) | EEPROM.read(LAST_X_ADDRESS+1);
}

void read_last_z_coordinate() {
  STATE_Z = ((EEPROM.read(LAST_Z_ADDRESS))<<8) | EEPROM.read(LAST_Z_ADDRESS+1);
}

ISR(TIMER1_OVF_vect) {
  #ifdef DEBUG_PROGRAM_FLOW_ON
    Serial.println("2");
  #endif

  if (i_command_time || command_time) { //Dwell
    if (i_command_time==1 && command_time) {
      ICR1 = (62500L*command_time/100)-1; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*command_time/(256*100))-1 = (62500Hz*command_time/100)-1
      if (ICR1<TCNT1) {
        TCNT1=0; //Checks if Timer already overrun the compare value
        //set Interrupt flag???
      }
    }
    else if ((!i_command_time && command_time) || (i_command_time==1 && !command_time)) {
        //If time is over
        TCCR1B = 0; //Disable Timer
        //Disable OVF1 Interrupt Enable
        TIMSK1 &= ~(_BV(TOIE1)); //set 0
        command_time=0;
        STATE2 &= ~(_BV(STATE2_COMMAND_TIME_BIT));
        //STATE_N++;
    }
    if (i_command_time) i_command_time--;
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
    }
    //Movement in +X-Direction
    else {
      if (current_x_step==3) {
        current_x_step=0;
      }
      else current_x_step++;
      x_step++;
    }
    
    //set next step
    set_xstep(current_x_step);

    //set coordinate
    STATE_X = get_xz_coordinates(X0, x_step);

    if (x_step==x_steps) { //last step reached?
      phi_x=0;
      x_step=0;
      x_steps=0;
      STATE2 &= ~(_BV(STATE2_XSTEPPER_RUNNING_BIT));
      x_command_completed=1;
      //Disable OVF1 Interrupt Enable
      TIMSK1 &= ~(_BV(TOIE1)); //set 0
      TCCR1B = 0; //Disable Timer
    }
    else { //next Timer-Compare-Value
      if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE || interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
        //Circular Interpolation with different speed settings for x- and z-stepper
        //Steps have to be seperated in max. 90 sections of same moving average feed.
        //For each of x_steps and z_steps an average phi of the section has to be calculated.
        //Maybe an calculation of the next phi with a modified Bresenham-Algorithm could improve it.
        
        //next X-Step moving average feed
        long phi_x_fixp = ((((long)x_step)*90+45)<<9)/x_steps; //max 22 bit used with X=32700 Fixpoint-Format => Q22.9
        //Rounding
        if ((phi_x_fixp%512) < 256) {
          phi_x = phi_x_fixp>>9;
        }
        else {
          phi_x = (phi_x_fixp>>9)+1;
        }
        if (phi_x == 0) { //phi_x has to be greater zero
          phi_x = 1;
        }
        
        if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
          //calculation of next x-clk (Direction)
          if ((z_steps<0)==(x_steps<0)) {
            clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_x))>>15;
            //clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
          }
          else {
            clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_x))>>15;
            //clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
          }
        }
        else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
          //calculation of next x-clk (Direction)
          if ((z_steps<0)==(x_steps<0)) {
            clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_x))>>15;
            //clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
          }
          else {
            clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_x))>>15;
            //clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
          }
        }
        
        //next Timer-Compare-Value
        //every step hast to be executed, feed can't be zero
        if (clk_xfeed) { //clock not zero
          ICR1 = (3750000L/clk_xfeed)-1; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*60(min/s)/(256*clk_xfeed))-1 = (62500Hz*60(min/s)/clk_xfeed)-1
          //Overflow possible!!!
        }
        else ICR1 = 62499U;
      }
      
      else if (interpolationmode==RAPID_LINEAR_MOVEMENT) {
      //set Timer-Compare-Values
        if (x_steps) {
          if (x_step < x_steps/2) {
            if (ICR1>RAPID_MAX) {
              ICR1 = RAPID_MIN-x_step*10; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*60(min/s)/(256*clk_xfeed))-1 = (62500Hz*60(min/s)/499s)-1
              if (ICR1<RAPID_MAX) {
                ICR1=RAPID_MAX;
              }
            }
          }
          else if ((x_steps-x_step) < 17) {
            ICR1 = RAPID_MIN-(x_steps-x_step)*10; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*60(min/s)/(256*clk_xfeed))-1 = (62500Hz*60(min/s)/499s)-1
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

  #ifdef DEBUG_PROGRAM_FLOW_ON
    Serial.println("3");
  #endif

  //next step in direction
  //Movement in -Z-Direction
  if (z_steps < 0) {
    if (current_z_step==0) {
      current_z_step=3;
    }
    else current_z_step--;
    z_step--;
  }
  //Movement in +Z-Direction
  else {
    if (current_z_step==3) {
      current_z_step=0;
    }
    else current_z_step++;
    z_step++;
  }

  //set next step
  set_zstep(current_z_step);

  //set coordinate
  STATE_Z = get_xz_coordinates(Z0, z_step);

  if (z_step==z_steps) { //last step reached?
    phi_z=0;
    z_step=0;
    z_steps=0;
    STATE2 &= ~(_BV(STATE2_ZSTEPPER_RUNNING_BIT));
    z_command_completed=1;
    //Disable OVF3 Interrupt Enable
    TIMSK3 &= ~(_BV(TOIE3)); //set 0
    TCCR3B = 0; //Disable Timer
  }
  else { //next Timer-Compare-Value
    if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE || interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
      //Circular Interpolation with different speed settings for x- and z-stepper
      //Steps have to be seperated in max. 90 sections of same moving average feed.
      //For each of x_steps and z_steps an average phi of the section has to be calculated.
      //Maybe an calculation of the next phi with a modified Bresenham-Algorithm could improve it.
      
      //next Z-Step moving average feed
      long phi_z_fixp = ((((long)z_step)*90+45)<<9)/z_steps; //max 22 bit used with Z=32700 Fixpoint-Format => Q22.9
      //Rounding
      if ((phi_z_fixp%512) < 256) {
        phi_z = phi_z_fixp>>9;
      }
      else {
        phi_z = (phi_z_fixp>>9)+1;
      }
      if (phi_z == 0) { //phi_z has to be greater zero
        phi_z = 1;
      }
      
      if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
        //calculation of next z-clk (Direction)
        if ((z_steps<0)==(x_steps<0)) {
          clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_z))>>15;
          //clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
        }
        else {
          clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_z))>>15;
          //clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
        }
      }
      else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
        //calculation of next z-clk (Direction)
        if ((z_steps<0)==(x_steps<0)) {
          clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_z))>>15;
          //clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
        }
        else {
          clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_z))>>15;
          //clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
        }
      }
      
      //next Timer-Compare-Value
      //every step hast to be executed, feed can't be zero
      if (clk_zfeed) { //clock not zero
        ICR3 = (3750000L/clk_zfeed)-1; //ICR3 = (16MHz*60(min/s)/(Prescaler*F_ICF3))-1 = (16MHz*60(min/s)/(256*clk_zfeed))-1 = (62500Hz*60(min/s)/clk_zfeed)-1
        //Overflow possible!!!
      } else ICR3 = 62499U;
    }

    else if (interpolationmode==RAPID_LINEAR_MOVEMENT) {
    //set Timer-Compare-Values
      if (z_steps) {
        if (z_step < z_steps/2) {
          if (ICR3>RAPID_MAX) {
            ICR3 = RAPID_MIN-z_step*10; //ICR1 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz*60(min/s)/(256*clk_zfeed))-1 = (62500Hz*60(min/s)/499s)-1
            if (ICR3<RAPID_MAX) {
              ICR3=RAPID_MAX;
            }
          }
        } else if ((z_steps-z_step) < 17) {
          ICR3 = RAPID_MIN-(z_steps-z_step)*10; //ICR1 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz*60(min/s)/(256*clk_zfeed))-1 = (62500Hz*60(min/s)/499s)-1
          if (ICR3>RAPID_MIN) {
            ICR3=RAPID_MIN;
          }
        }
      }
    }
  }
  //reset INTR-flag? OVF resets automatically
}

