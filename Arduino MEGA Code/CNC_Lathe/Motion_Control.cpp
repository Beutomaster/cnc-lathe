#include "Motion_Control.h"

boolean absolute=0, feed_modus=0;
volatile byte interpolationmode=0, i_command_time=0;
volatile int command_time=0;
volatile int X0=0, Z0=0;


void set_xz_coordinates(int x_origin, int z_origin) {
  STATE_X -= x_origin;
  STATE_Z -= z_origin;
}

void set_x_coordinate(int x_origin) {
  STATE_X -= x_origin;
}

void set_z_coordinate(int z_origin) {
  STATE_Z -= z_origin;
}

int get_inc_X(int abs_X) { //get incremental x-Coordinate
  return abs_X-STATE_X;
}

int get_inc_Z(int abs_Z) { //get incremental z-Coordinate
  return abs_Z-STATE_Z;
}

int get_Tool_X(int TOOL_X) { //get Tool x-Coordinate
  return TOOL_X-STATE_X;
}

int get_Tool_Z(int Tool_Z) { //get Tool z-Coordinate
  return Tool_Z-STATE_Z;
}

void set_xz_move(int X, int Z, int feed, byte local_interpolationmode) {
  //int x_steps=0; //has to be global for ISR
  //int z_steps=0; //has to be global for ISR
  //int x_feed=0; //has to be global for ISR
  //int z_feed=0; //has to be global for ISR
  //int command_time=0;
  command_completed=0;
  
  if (feed_modus==FEED_IN_MM_PER_REVOLUTION) {
    STATE_F =  get_xz_feed_related_to_revolutions(feed);
  } else STATE_F = feed;
  
  interpolationmode=local_interpolationmode;

  //turn stepper on with last step
  if (!((STATE1>>STATE1_STEPPER_BIT)&1)) stepper_on();

  X0 = STATE_X;
  Z0 = STATE_Z;
  
  //get incremental coordinates
  if (absolute){
    X=get_inc_X(X);
    Z=get_inc_Z(Z);
  }

  //end if nothing to do
  if (!X && !Z) {
    command_completed=1;
    return;
  }

  //calculate needed steps
  x_steps = (long)X*STEPS_PER_MM/100;
  z_steps = (long)Z*STEPS_PER_MM/100;

  
  #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_STEPPER_ON
    //#error Stepper debug-msg compilation activated!
    Serial.print(F("XStepper starts moving "));
    Serial.print(x_steps, DEC);
    Serial.println(F("Steps"));
    Serial.print(F("ZStepper starts moving "));
    Serial.print(z_steps, DEC);
    Serial.println(F("Steps"));
  #endif

  clk_feed = (long)STATE_F * STEPS_PER_MM; //clk_feed in Steps/min

  //Prepare Timer 1 for X-Stepper 
  TCCR1B = 0b00011000; //connect no Input-Compare-PINs, WGM13=1, WGM12=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
  TCCR1A = 0b00000010; //connect no Output-Compare-PINs and WGM11=1, WGM10=0 for Fast PWM with ICR1=TOP
  TCCR1C = 0; //no Force of Output Compare
  
  //Prepare Timer 3 for Z-Stepper
  TCCR3B = 0b00011000; //connect no Input-Compare-PINs, WGM33=1, WGM32=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
  TCCR3A = 0b00000010; //connect no Output-Compare-PINs and WGM31=1, WGM30=0 for Fast PWM with ICR3=TOP
  TCCR3C = 0; //no Force of Output Compare

  if (interpolationmode==INTERPOLATION_LINEAR) {
    if (Z==0) {
      x_feed=STATE_F;
      z_feed=0;
    } else {
      x_feed=(long)X*STATE_F/((long)Z+(long)X);
      if (x_feed==0) x_feed=1; //Minimum needed
    }
    if (X==0) {
      x_feed=0;
      z_feed=STATE_F;
    } else {
      z_feed=(long)Z*STATE_F/((long)Z+(long)X);
      if (z_feed==0) z_feed=1; //Minimum needed
    }

    clk_xfeed = (long)x_feed * STEPS_PER_MM; //clk_xfeed in Steps/min
    clk_zfeed = (long)z_feed * STEPS_PER_MM; //clk_xfeed in Steps/min

    //set Timer-Compare-Values
    if (X) {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR1 = (3750000L/clk_xfeed)-1; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*60(s/min)/(256*clk_xfeed))-1 = (62500Hz*60(s/min)/clk_xfeed)-1
      }
    }
    if (Z) {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR3 = (3750000L/clk_zfeed)-1; //ICR3 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz*60(s/min)/(256*clk_zfeed))-1 = (62500Hz*60(s/min)/clk_zfeed)-1
      }
    }
  }

  else if (interpolationmode==RAPID_LINEAR_MOVEMENT) {
    //set Timer-Compare-Values
    if (X) {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR1 = RAPID_MIN; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*60(s/min)/(256*clk_xfeed))-1 = (62500Hz*60(s/min)/(499(mm/min)*STEPS_PER_MM))-1
      }
    }
    if (Z) {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR3 = RAPID_MIN; //ICR3 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz*60(s/min)/(256*clk_zfeed))-1 = (62500Hz*60(s/min)/(499(mm/min)*STEPS_PER_MM))-1
      }
    }
  }
  
  else { //Circular Interpolation with different speed settings for x- and z-stepper
    //Steps have to be seperated in max. 90 sections of same moving average feed.
    //For each of x_steps and z_steps an average phi of the section has to be calculated.
    //Maybe an calculation of the next phi with a modified Bresenham-Algorithm could improve it.
    
    //next X- and Z-Step moving average feed
    if (x_steps) {
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
    }
    if (z_steps) { 
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
    }
    
    if (interpolationmode==INTERPOLATION_CIRCULAR_CLOCKWISE) {
      //calculation of next x- and z-clk (Direction)
      if ((z_steps<0)==(x_steps<0)) {
        clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_x))>>15;
        clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_z))>>15;
        //clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
        //clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
      }
      else {
        clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_x))>>15;
        clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_z))>>15;
        //clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
        //clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
      }
    }
    else if (interpolationmode==INTERPOLATION_CIRCULAR_COUNTERCLOCKWISE) {
      //calculation of next x- and z-clk (Direction)
      if ((z_steps<0)==(x_steps<0)) {
        clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_x))>>15;
        clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_z))>>15;
        //clk_xfeed = (clk_feed * lookup_cosinus[phi_x])>>15;
        //clk_zfeed = (clk_feed * lookup_cosinus[90-phi_z])>>15;
      }
      else {
        clk_xfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+90-phi_x))>>15;
        clk_zfeed = (clk_feed * pgm_read_word_near(lookup_cosinus+phi_z))>>15;
        //clk_xfeed = (clk_feed * lookup_cosinus[90-phi_x])>>15;
        //clk_zfeed = (clk_feed * lookup_cosinus[phi_z])>>15;
      }
    }
    
    //set Timer-Compare-Values
    //every step has to be executed, feed can't be zero
    if (clk_xfeed) { //clock not zero
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR1 = (3750000L/clk_xfeed)-1; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*60(s/min)/(256*clk_xfeed))-1 = (62500Hz*60(s/min)/clk_xfeed)-1 
        //Overflow possible!!!
      }
    } else {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR1 = 62499U;
      }
    }
    if (clk_zfeed) { //clock not zero
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR3 = (3750000L/clk_zfeed)-1; //ICR3 = (16MHz/(Prescaler*F_ICF3))-1 = (16MHz*60(s/min)/(256*clk_zfeed))-1 = (62500Hz*60(s/min)/clk_zfeed)-1
        //Overflow possible!!!
      }
    } else {
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR3 = 62499U;   
      }
    }
  }

  #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_STEPPER_ON
    //#error Stepper debug-msg compilation activated!
    Serial.print(F("XStepper clk_xfeed "));
    Serial.println(clk_xfeed, DEC);
    Serial.print(F("ZStepper clk_zfeed "));
    Serial.println(clk_zfeed, DEC);
    Serial.print(F("XStepper ICR1 "));
    Serial.println(ICR1, DEC);
    Serial.print(F("ZStepper ICR3 "));
    Serial.println(ICR3, DEC);
  #endif
  
  //start Timer
  if (X) {
    STATE2 |= _BV(STATE2_XSTEPPER_RUNNING_BIT);
    x_command_completed=0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      TCNT1 = 0; //set Start Value
    }
    TIFR1 = _BV(TOV1); //clear Interrupt flag by writing a logical one to it's bit, zeros don't alter the register
    //OVF1 Interrupt Enable
    TIMSK1 |= _BV(TOIE1); //set 1
    //Prescaler 256 and Start Timer
    TCCR1B |= _BV(CS12); //set 1
  }
  if (Z) {
    STATE2 |= _BV(STATE2_ZSTEPPER_RUNNING_BIT);
    z_command_completed=0;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      TCNT3 = 0; //set Start Value
    }
    TIFR3 = _BV(TOV3); //clear Interrupt flag by writing a logical one to it's bit, zeros don't alter the register
    //OVF3 Interrupt Enable
    TIMSK3 |= _BV(TOIE3); //set 1
    //Prescaler 256 and Start Timer
    TCCR3B |= _BV(CS32); //set 1
  }
}

static inline int get_xz_coordinates(int XZ0, int xz_step) { //calculate Coordinates (avr-gcc ignores "inline" with default Optimizationlevel "-Os" used by Arduino-IDE. use -O3 instead.)
  int XZ_delta;
  long XZ_delta_fixpoint = (((long)xz_step*100)<<9)/STEPS_PER_MM; //max 22 bit used with Z=32700 Fixpoint-Format => Q22.9
      //Rounding
      if ((XZ_delta_fixpoint%512) < 256) {
        XZ_delta = XZ_delta_fixpoint>>9;
      }
      else {
        XZ_delta = (XZ_delta_fixpoint>>9)+1;
      }
  return XZ0 + XZ_delta;
}

int get_xz_feed() {
	int feed=0; //Stub
	return feed;
}

int get_xz_feed_related_to_revolutions(int feed_per_revolution) { // for G95 - Feed in mm/rev.
  int feed = feed_per_revolution*STATE_RPM;
  return feed;
}


//should be replaced by command_completed=1; in Stepper and Toolchanger ISR
//still needed for G04

void command_running(int local_command_time) { //command_time in 1/100s
  //handling durations over 1s
  i_command_time = 1+(local_command_time/100);
  command_time = local_command_time%100;

  //set and start Timer1 for command_time
  TCCR1B = 0b00011000; //connect no Input-Compare-PINs, WGM13=1, WGM12=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
  TCCR1A = 0b00000010; //connect no Output-Compare-PINs and WGM11=1, WGM10=0 for Fast PWM with ICR1 as TOP
  TCCR1C = 0; //no Force of Output Compare
    
  if (i_command_time) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      ICR1 = 62499U; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*command_time/(256*100))-1 = (62500Hz*100s/100)-1
    }
  }
  else {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      ICR1 = (62500L*command_time/100)-1; //ICR1 = (16MHz/(Prescaler*F_ICF1))-1 = (16MHz*command_time/(256*100))-1 = (62500Hz*command_time/100)-1
    }
  }
  if (i_command_time || command_time) {
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      TCNT1 = 0; //set Start Value
    }
    command_completed=0;
    STATE2 |= _BV(STATE2_COMMAND_TIME_BIT);
    TIFR1 = _BV(TOV1); //clear Interrupt flag by writing a logical one to it's bit, zeros don't alter the register
    //OVF Interrupt Enable
    TIMSK1 |= _BV(TOIE1); //set 1
    //Prescaler 256 and Start Timer
    TCCR1B |= _BV(CS12); //set 1
  }
}

