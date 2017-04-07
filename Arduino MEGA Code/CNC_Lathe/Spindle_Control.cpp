#include "Spindle_Control.h"

//global ISR vars
volatile unsigned long delta_rpm_time=0, last_rpm_time=0;
volatile int max_revolutions=REVOLUTIONS_MAX, target_revolutions=0;
volatile char rpm_measurement_active=0, wait_for_spindle_start=0, wait_for_spindle_stop=0, callback_spindle_direction_change=0, target_spindle_on=0, target_spindle_direction=0, wait_for_spindle_spindle_direction_relais=0, callback_spindle_start=0;
#ifdef SPINDLEDRIVER_NEW
  volatile int delta_revolution_last=0;
  volatile long y=0, y_last=0;
  volatile boolean spindle_new=HIGH;
#endif
volatile int adcvalue = 0; // adc value read by ISR(ADC_vect)
volatile uint8_t adcpin = 0; // actual adc-pin for intr_analogRead
#ifdef SPINDLE_STATE_CODE_NEW
  volatile unsigned int rpm_count=0;
#endif

//other global vars
#ifdef SPINDLEDRIVER_NEW
  char spindle_state = SPINDLE_STATE_CHARGE_RESISTOR_ON, spindle_command_completed=0;
#else
  char spindle_state = SPINDLE_STATE_SPINDLE_OFF, spindle_command_completed=1;
#endif
#ifdef SPINDLE_STATE_CODE_NEW
  unsigned long spindle_wait_timestamp=0;
#endif

#ifdef SERVO_LIB
  //Create new Servo Objekt
  Servo potiservo;  //old Servo Bib
#endif

void spindle_on() {
  if (!((ERROR_NO>>ERROR_SPINDLE_BIT)&1)) {
    #ifndef SPINDLE_STATE_CODE_NEW
      if (!callback_spindle_direction_change && !wait_for_spindle_spindle_direction_relais && !command_time && !i_command_time && !i_tool && x_command_completed && z_command_completed) {
        last_rpm_time = micros();
        digitalWrite(PIN_SPINDLE_ON, HIGH);
        STATE1 |= _BV(STATE1_SPINDLE_BIT); //set STATE1_bit5 = spindle
        //wait until spindle is turning with target-RPM (time could be shortend by comparing STATE_RPM with target_RPM)
        wait_for_spindle_start=1; //is resetted in main
        command_running(SPINDLE_ON_WAIT_TIME);
      }
      else callback_spindle_start=1; //is resetted in main
    #else
      if (target_spindle_on != 1) {
        spindle_command_completed=0;
        target_spindle_on = 1;
      }
    #endif
  }
}

void spindle_off() {
  #ifndef SPINDLE_STATE_CODE_NEW
    if ((STATE1>>STATE1_SPINDLE_BIT)&1) {
      digitalWrite(PIN_SPINDLE_ON, LOW);
      STATE1 &= ~(_BV(STATE1_SPINDLE_BIT)); //delete STATE1_bit5 = spindle
      wait_for_spindle_stop=1; //is resetted in main
      command_running(SPINDLE_OFF_WAIT_TIME); //wait a few seconds until spindle has stopped! (needed for a secure change of spindle-direction) do not trust STATE_RPM-measurement!!!
    }
  #else
    if (target_spindle_on != 0) {
      spindle_command_completed=0;
      target_spindle_on = 0;
    }
  #endif
}

boolean test_for_spindle_off() {
  #ifndef BOARDVERSION_1_25
    if (!digitalRead(PIN_SPINDLE_ON_DETECT) && !STATE_RPM) { //maybe STATE_RPM<10
    //if (!digitalRead(PIN_SPINDLE_ON_DETECT) && STATE_RPM<10) {
      return true;
    }
  #else
    if (!STATE_RPM) { //maybe STATE_RPM<10
      return true;
    }
  #endif
    else {
      #ifndef SPINDLE_STATE_CODE_NEW
        //if spindle is still turning after wait time, set Spindle-Error
        if (!((STATE1>>STATE1_SPINDLE_BIT)&1) && !wait_for_spindle_stop) ERROR_NO |= _BV(ERROR_SPINDLE_BIT);
      #endif
      return false;
    }
}

void get_spindle_state_passiv() {
  #ifndef BOARDVERSION_1_25
    if (digitalRead(PIN_SPINDLE_ON_DETECT)) {
      STATE1 |= _BV(STATE1_SPINDLE_BIT); //set STATE1_bit5 = spindle
    }
  #else
    if (STATE_RPM) {
      STATE1 |= _BV(STATE1_SPINDLE_BIT); //set STATE1_bit5 = spindle
    }
  #endif
    else {
      STATE1 &= ~(_BV(STATE1_SPINDLE_BIT)); //delete STATE1_bit5 = spindle
    }
}

#ifndef SPINDLE_STATE_CODE_NEW
void spindle_direction(boolean spindle_reverse) {
  //don't change the spindle-direction, while the spindle is turning!!! Turn the Spindle-Switch of Emco Control off, before avtivate or deactivate new control!!
  if (!((ERROR_NO>>ERROR_SPINDLE_BIT)&1)) {
    target_spindle_direction=spindle_reverse;
    if (!((STATE1>>STATE1_SPINDLE_BIT)&1) && !wait_for_spindle_stop) { //for security
      if (test_for_spindle_off()) {
        if (spindle_reverse) {
          #ifndef SPINDLEDRIVER_NEW
            digitalWrite(PIN_SPINDLE_DIRECTION, HIGH);
          #else
            if (spindle_new) { //Hotfix for Board V1.25, should be changed in V2.1
              digitalWrite(PIN_SPINDLE_DIRECTION, LOW);
            }
            else {
              digitalWrite(PIN_SPINDLE_DIRECTION, HIGH);
            }
          #endif
          STATE1 |= _BV(STATE1_SPINDLE_DIRECTION_BIT); //set STATE1_bit6 = spindle_direction
        }
        else {
          #ifndef SPINDLEDRIVER_NEW
            digitalWrite(PIN_SPINDLE_DIRECTION, LOW);
          #else
            if (spindle_new) { //Hotfix for Board V1.25, should be changed in V2.1
              digitalWrite(PIN_SPINDLE_DIRECTION, HIGH);
            }
            else {
              digitalWrite(PIN_SPINDLE_DIRECTION, LOW);
            }
          #endif
          STATE1 &= ~(_BV(STATE1_SPINDLE_DIRECTION_BIT)); //delete STATE1_bit6 = spindle_direction
        }
        wait_for_spindle_spindle_direction_relais=1;
        command_running(RELAIS_WAIT_TIME); //wait time needed!!! Relais needs a few ms
      }
    } //end of if (!((STATE1>>STATE1_SPINDLE_BIT)&1) && !wait_for_spindle_stop)
    else if (spindle_reverse != (STATE1>>STATE1_SPINDLE_DIRECTION_BIT)&1) { //maybe better to change it only when needed, but not usable with Hotfix for Board V1.25, should be changed in V2.1
      callback_spindle_direction_change=1; //is resetted in main
      spindle_off();
    }
    else {
      callback_spindle_direction_change=0; //no change needed, maybe not possible with Board V1.25!!!
      #ifdef SPINDLEDRIVER_NEW
        if (hotfix_V1_25_force_spindle_direction_switch) {
          hotfix_V1_25_force_spindle_direction_switch=0;
          callback_spindle_direction_change=1; //is resetted in main
          spindle_off();
        }
      #endif
    }
  } //end of if (!((ERROR_NO>>ERROR_SPINDLE_BIT)&1))
}
#else
void spindle_direction(boolean spindle_reverse) {
  if (spindle_reverse != target_spindle_direction) {
    spindle_command_completed=0;
    target_spindle_direction = spindle_reverse;
  }
}

void spindle_direction_private(boolean spindle_reverse) {
  //don't change the spindle-direction, while the spindle is turning!!! Turn the Spindle-Switch of Emco Control off, before avtivate or deactivate new control!!
  if (spindle_reverse) {
    #ifndef SPINDLEDRIVER_NEW
      digitalWrite(PIN_SPINDLE_DIRECTION, HIGH);
    #else
      if (spindle_new) { //Hotfix for Board V1.25, should be changed in V2.1
        digitalWrite(PIN_SPINDLE_DIRECTION, LOW);
      }
      else {
        digitalWrite(PIN_SPINDLE_DIRECTION, HIGH);
      }
    #endif
    STATE1 |= _BV(STATE1_SPINDLE_DIRECTION_BIT); //set STATE1_bit6 = spindle_direction
  }
  else {
    #ifndef SPINDLEDRIVER_NEW
      digitalWrite(PIN_SPINDLE_DIRECTION, LOW);
    #else
      if (spindle_new) { //Hotfix for Board V1.25, should be changed in V2.1
        digitalWrite(PIN_SPINDLE_DIRECTION, HIGH);
      }
      else {
        digitalWrite(PIN_SPINDLE_DIRECTION, LOW);
      }
    #endif
    STATE1 &= ~(_BV(STATE1_SPINDLE_DIRECTION_BIT)); //delete STATE1_bit6 = spindle_direction
  }
}
#endif

void set_spindle_state_spindle_on() {
  STATE1 |= _BV(STATE1_SPINDLE_BIT); //set STATE1_bit5 = spindle
  spindle_state=SPINDLE_STATE_SPINDLE_ON;
}

void set_spindle_state_spindle_off() {
  STATE1 &= ~(_BV(STATE1_SPINDLE_BIT)); //delete STATE1_bit5 = spindle
  spindle_state=SPINDLE_STATE_SPINDLE_OFF;
}

void set_spindle_state_spindle_error() {
  digitalWrite(PIN_SPINDLE_ON, LOW);
  ERROR_NO |= _BV(ERROR_SPINDLE_BIT);
  spindle_state=SPINDLE_STATE_SPINDLE_ERROR;
}


void set_revolutions(int target_revolutions_local) {

  //for G196, maybe only at G96
  if (feed_modus==FEED_IN_M_PER_MIN_AT_INCR_REVOLUTIONS) {
    if (target_revolutions_local>max_revolutions) {
      target_revolutions_local = max_revolutions;
    }
  }
  
	//Poti-Servo
	//int poti_angle = map(target_revolutions_local, REVOLUTIONS_MIN, REVOLUTIONS_MAX, 0, 180);
	//set_poti_servo(poti_angle);
  set_poti_servo_revolutions(target_revolutions_local);

  #ifdef SPINDLEDRIVER_EXTRA_BOARD
  	//Alternative send RPM over UART1 to Niko's Spindle-Board (0 to 255)
    byte rev_niko=map(target_revolutions_local, 0, REVOLUTIONS_MAX, 0, 255);
    Serial1.write (rev_niko);
  #endif

  target_revolutions = target_revolutions_local;

  #if defined BOARDVERSION_1_25 && defined SPINDLEDRIVER_NEW
    //Timer4 Fast PWM (OC4C) for Niko's spindle driver (set Revolutions)
    //min. 16KHz?
    //230V AC * sqrt(2) => ca. 325 V DC - deltaU/2 (Glättung)
    //Motor max. 180 V DC * 100 /325 V DC = 55,384615384615384615384615384615 %
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
      OCR4C = map(target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR4C_min, OCR4C_max);
      TCNT4 = 0; //set Start Value
    }
  #endif

  //Debug
  #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_RPM_ON
    //#error RPM debug-msg compilation activated!
    Serial.print(F("RPM-set-Value: "));
    Serial.println (target_revolutions_local);
  #endif
}

int get_SERVO_CONTROL_POTI() {
	//Reading Poti-Value
	int manual_target_revolutions = analogRead(APIN_SERVO_CONTROL_POTI);
	
	//Convert 10-bit Value from Analog-Input (0-1023) to 460-3220 rpm
	target_revolutions = map(manual_target_revolutions, 0, 1023, REVOLUTIONS_MIN, REVOLUTIONS_MAX);

  //needs moving average calculation

	return target_revolutions;
}

void set_poti_servo(int poti_angle){
  #ifdef SERVO_LIB
  	//write angle in degree to Servo-Objekt
  	potiservo.write(poti_angle); //old Servo Lib
  #else
    OCR5A = (OCR5A_max-OCR5A_min)*poti_angle/180 + OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler = 544µs*16MHz/8 = 1088 ... OCR5A = 2400µs*16MHz/8 = 4800
  #endif
}

void set_poti_servo_revolutions(int local_target_revolutions){
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    //OCR5A = map(local_target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR5A_min, OCR5A_max);
    OCR5A = OCR5A_max + OCR5A_min - map(local_target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR5A_max, OCR5A_min);
    //OCR5A = (OCR5A_max-OCR5A_min)*(local_target_revolutions-REVOLUTIONS_MIN)/(REVOLUTIONS_MAX-REVOLUTIONS_MIN) + OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler = 544µs*16MHz/8 = 1088 ... OCR5A = 2400µs*16MHz/8 = 4800
  }
}

void get_revolutions_ISR() { //read revolution-sensor
  //first value may be wrong at passive modus, overflow of timer0 may cause errors
  //We should change the PIN with PIN_REVOLUTIONS_COUNT in V2!!! Better resolution!!!
  //needs moving average calculation
  #ifndef SPINDLE_STATE_CODE_NEW
    rpm_time = micros();
    #ifndef BOARDVERSION_1_25
      STATE_RPM = 600000UL/(rpm_time-last_rpm_time); //numerator for Board V3.2 with Count-Signal: 1U*(60000000us/min)/(100"Count"/U) = 600000 U*us/min, should be done in main (moving average calculation)!!!
    #else
      STATE_RPM = 60000000UL/(rpm_time-last_rpm_time); //numerator for Board V1.25 with Sync-Signal: 1U*(60000000us/min)/(1"Sync"/U) = 60000000 U*us/min, should be done in main (moving average calculation)!!!
    #endif
    last_rpm_time = rpm_time;
  #else
    if (rpm_count == 0) {
      last_rpm_time = micros();
      rpm_count++;
    }
    #ifndef BOARDVERSION_1_25
    else if (rpm_count == 100) {
    #else
    else if (rpm_count == 10)) {
    #endif
      rpm_count = 0;
      delta_rpm_time = last_rpm_time - micros();
      rpm_measurement_active = 1;
    }
    else rpm_count++;
  #endif
}

boolean test_for_spindle_rpm(int target_rpm, int range) {
  if (STATE_RPM>target_rpm-range || STATE_RPM<target_rpm+range) return true;
  else return false;
}

#ifdef SPINDLEDRIVER_NEW
  void set_spindle_new(boolean spindle_new_local){
    spindle_new = spindle_new_local;
    spindle_off();
    set_Timer5();
    //waiting time needed!!!!
    if (spindle_new) {
      digitalWrite(PIN_SPINDLE_NEW, HIGH);
    }
    else {
      digitalWrite(PIN_SPINDLE_NEW, LOW);
    }
    hotfix_V1_25_force_spindle_direction_switch=1;
    spindle_direction((STATE1>>STATE1_SPINDLE_DIRECTION_BIT)&1); //Hotfix for Board V1.25, should be changed in V2.1
  }
#endif

//Timer5 Servo and spindle regulator
void set_Timer5 () {
  #ifdef SPINDLEDRIVER_NEW
    if (spindle_new) { //spindle regulator
      //set and start Timer5 with 1ms TOP
      TCCR5B = 0b00011000; //connect no Input-Compare-PINs, WGM53=1, WGM52=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
      TCCR5A = 0b00000010; //WGM51=1, WGM50=1 for Fast PWM with ICR5=TOP
      TCCR5C = 0; //no Force of Output Compare
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        ICR5 = 1999; //ICR5 = T_ICR5*16MHz/Prescaler -1 = 1000µs*16MHz/8 -1 = 1999
        TCNT5 = 0; //set Start Value
      }
      //Overflow Interrupt Enable
      TIMSK5 |= _BV(TOIE5); //set 1
      //Prescaler 8 and Start Timer
      TCCR5B |= _BV(CS51); //set 1
    }
    else { //Servo
      //set and start Timer5 with 20ms TOP and 544µs to 2400µs OCR5A
      TCCR5B = 0b00011000; //connect no Input-Compare-PINs, WGM53=1, WGM52=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
      TCCR5A = 0b10000010; //clear OC5A-PIN (PIN 46) with COM5A1=1 and COM5A=0 at Output Compare and WGM51=1, WGM50=0 for Fast PWM with ICR5=TOP
      TCCR5C = 0; //no Force of Output Compare
      ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //maybe better seperated
        ICR5 = 39999; //ICR5 = T_ICR5*16MHz/Prescaler -1 = 20ms*16MHz/8 -1 = 39999
        OCR5A = OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler -1 = 544µs*16MHz/8 -1 = 1091
        TCNT5 = 0; //set Start Value
      }
      //Overflow Interrupt Disable
      TIMSK5 &= ~(_BV(TOIE5)); //delete bit0
      //Prescaler 8 and Start Timer
      TCCR5B |= _BV(CS51); //set 1
    }
  #else
    //Servo
    //set and start Timer5 with 20ms TOP and 544µs to 2400µs OCR5A
    TCCR5B = 0b00011000; //connect no Input-Compare-PINs, WGM53=1, WGM52=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
    TCCR5A = 0b10000010; //clear OC5A-PIN (PIN 46) with COM5A1=1 and COM5A=0 at Output Compare and WGM51=1, WGM50=0 for Fast PWM with ICR5=TOP
    TCCR5C = 0; //no Force of Output Compare
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //maybe better seperated
      ICR5 = 39999; //ICR5 = T_ICR5*16MHz/Prescaler -1 = 20ms*16MHz/8 -1 = 39999
      OCR5A = OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler -1 = 544µs*16MHz/8 -1 = 1091
      TCNT5 = 0; //set Start Value
    }
    //Overflow Interrupt Disable
    TIMSK5 &= ~(_BV(TOIE5)); //delete bit0
    //Prescaler 8 and Start Timer
    TCCR5B |= _BV(CS51); //set 1
  #endif
}

#ifdef SPINDLEDRIVER_NEW
  //spindle regulator
  ISR(TIMER5_OVF_vect){
      //PI-Regulator
      //get Regulator-Parameter for 15,625kHz with 20 to 80% PWM and Ziegler-Nicols formula
      //#define K_P 1; //0,001 - 100 ???
      //#define KI_TN 1; //1s
      //T_Regler=1ms
    /*
      int delta_revolution = STATE_RPM - target_revolutions;
      y = y_last + (((int32_t)K_P*(delta_revolution - delta_revolution_last))>>15) + (((int32_t)KI_TN * delta_revolution)>>15);
      OCR4C = map(y, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR4C_min, OCR4C_max);
      y_last = y;
      delta_revolution_last = delta_revolution;
    */
  }
#endif


void intr_analogRead(uint8_t pin) {
  //function is compatible with analogRead(uint8_t pin), when used after adc-interrupt was disabled in ISR(ADC_vect)
  //adcvalue has the same range (0-1023 for GND-5V_Arduino) as the return-value of analogRead
  //configure the pin which is read by the adc (ADC0 - ADC15 at Arduino Mega 2560)
  
  // disable digital input buffer for pin (if pin is used as digital input afterwards, is has to be enabled again)
  if (pin<=7) DIDR0 &= ~_BV(pin);
  else if (pin<=15) DIDR2 &= ~_BV(pin-8);
  else return; //abort if pin-number wrong

  adcpin = pin;
  
  /*
    // this is done by init() in arduino-main()
    // set a2d prescaler so we are inside the desired 50-200 KHz range.
    #if F_CPU >= 16000000 // 16 MHz / 128 = 125 KHz
    sbi(ADCSRA, ADPS2);
    sbi(ADCSRA, ADPS1);
    sbi(ADCSRA, ADPS0);
    // enable a2d conversions
    sbi(ADCSRA, ADEN);
  */
  
  #if defined(ADCSRB) && defined(MUX5)
    // the MUX5 bit of ADCSRB selects whether we're reading from channels
    // 0 to 7 (MUX5 low) or 8 to 15 (MUX5 high).
    ADCSRB = (ADCSRB & ~(1 << MUX5)) | (((pin >> 3) & 0x01) << MUX5);
  #endif
  ADMUX = _BV(REFS0) | (pin & 0x07); //REFS1=0,REFS0=1 (AREF=5V, AVCC with external capacitor at AREF pin), ADLAR=0 (ADC right Adjust Result), MUX4:0 = (pin & 0x07) (select PIN together with MUX5)
  
  //clear ADC-INTR-Flag by writing a one to ADIF (maybe analogRead was used before), enable ADC-INTR, start the conversion
  ADCSRA |= _BV(ADIF) | _BV(ADIE) | _BV(ADSC);
}


ISR(ADC_vect) {
  adcvalue = ADCL;        // ADC low-byte must be read first, to block update of Register
  adcvalue |= (ADCH<<8);
  //disable ADC-INTR
  ADCSRA &= ~(_BV(ADIE));

  //workaround-replacement for set_poti_servo_revolutions
  if (adcpin == APIN_SERVO_CONTROL_POTI) {
    //Convert 10-bit Value from Analog-Input (0-1023) to 460-3220 rpm
    target_revolutions = map(adcvalue, 0, 1023, REVOLUTIONS_MIN, REVOLUTIONS_MAX);
    OCR5A = OCR5A_max + OCR5A_min - map(target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR5A_max, OCR5A_min);
  }
}

