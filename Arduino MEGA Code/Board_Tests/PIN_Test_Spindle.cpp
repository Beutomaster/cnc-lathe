#include "PIN_Test_Spindle.h"


//global ISR vars
volatile unsigned long rpm_time=0, last_rpm_time=0;
volatile long y=0, y_last=0;
volatile int target_revolutions=0, delta_revolution_last=0;
volatile bool spindle_new=LOW;

//Create new Servo Objekt
//Servo potiservo;  //old Servo Lib

void spindle_on() {
  digitalWrite(PIN_SPINDLE_ON, HIGH);
}

void spindle_off() {
  digitalWrite(PIN_SPINDLE_ON, LOW);
  delay(5000);
}

void set_revolutions(int target_revolutions_local) {
  //Poti-Servo
  //int poti_angle = map(target_revolutions_local, REVOLUTIONS_MIN, REVOLUTIONS_MAX, 0, 180);
  //set_poti_servo(poti_angle);
  set_poti_servo(target_revolutions_local);
 
  //Debug
  Serial.print("RPM-set-Value: ");
  Serial.println (target_revolutions_local);
  
  //Timer4 Fast PWM (OC4C) for Niko's spindle driver (set Revolutions)
  //min. 16KHz?
  //230V AC * sqrt(2) => ca. 325 V DC - deltaU/2 (Glättung)
  //Motor max. 180 V DC * 100 /325 V DC = 55,384615384615384615384615384615 %
  target_revolutions = target_revolutions_local;
  OCR4C = map(target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR4C_min, OCR4C_max);
  TCNT4 = 0; //set Start Value
}

int get_SERVO_CONTROL_POTI() {
  //Reading Poti-Value
  int manual_target_revolutions = analogRead(APIN_SERVO_CONTROL_POTI);
  
  //Convert 10-bit Value from Analog-Input (0-1023) to 460-3220 rpm
  manual_target_revolutions = map(manual_target_revolutions, 0, 1023, REVOLUTIONS_MIN, REVOLUTIONS_MAX);

  return manual_target_revolutions;
}

/*
void set_poti_servo(int poti_angle){
  //write angle in degree to Servo-Objekt
  //potiservo.write(poti_angle); //old Servo Lib
  OCR5A = (OCR5A_max-OCR5A_min)*poti_angle/180 + OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler = 544µs*16MHz/8 = 1088 ... OCR5A = 2400µs*16MHz/8 = 4800
}
*/

void set_poti_servo(int local_target_revolutions){
  OCR5A = OCR5A_max + OCR5A_min - map(local_target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR5A_max, OCR5A_min);
  //OCR5A = (OCR5A_max-OCR5A_min)*(local_target_revolutions-REVOLUTIONS_MIN)/(REVOLUTIONS_MAX-REVOLUTIONS_MIN) + OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler = 544µs*16MHz/8 = 1088 ... OCR5A = 2400µs*16MHz/8 = 4800

  //Debug
    Serial.print("OCR5A:");
    Serial.println(OCR5A);
}

void set_spindle_new(bool spindle_new_local){
  spindle_new = spindle_new_local;
  spindle_off();
  if (spindle_new) {
    set_Timer5();
    digitalWrite(PIN_SPINDLE_NEW, HIGH);
  }
  else {
    set_Timer5();
    digitalWrite(PIN_SPINDLE_NEW, LOW);
  }
  //spindle_direction((STATE>>STATE_SPINDLE_DIRECTION_BIT)&1); //Hotfix for Board V1.25, should be changed in V2.1
}

//Timer5 Servo and spindle regulator
void set_Timer5 () {
  if (spindle_new) { //spindle regulator
    //set and start Timer5 with 1ms TOP
    TCCR5B = 0b00011000; //connect no Input-Compare-PINs, WGM53=1, WGM52=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
    TCCR5A = 0b00000011; //WGM51=0, WGM50=1 for Fast PWM with ICR5=TOP
    TCCR5C = 0; //no Force of Output Compare
    OCR5A = 1999; //OCR5A = T_OCF5A*16MHz/Prescaler -1 = 1000µs*16MHz/8 -1 = 1999
    TCNT5 = 0; //set Start Value
    //Prescaler 8 and Start Timer
    TCCR5B |= _BV(CS51); //set 1
  }
  else { //Servo
    //set and start Timer5 with 20ms TOP and 544µs to 2400µs OCR5A
    TCCR5B = 0b00011000; //connect no Input-Compare-PINs, WGM53=1, WGM52=1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
    TCCR5A = 0b10000010; //clear OC5A-PIN (PIN 46) with COM5A1=1 and COM5A=0 at Output Compare and WGM51=1, WGM50=0 for Fast PWM with ICR5=TOP
    TCCR5C = 0; //no Force of Output Compare
    ICR5 = 39999; //ICR5 = T_ICR5*16MHz/Prescaler -1 = 20ms*16MHz/8 -1 = 39999
    OCR5A = OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler -1 = 544µs*16MHz/8 -1 = 1091
    TCNT5 = 0; //set Start Value
    //Overflow Interrupt Enable
    TIMSK5 &= ~(_BV(TOIE5)); //delete bit0
    //Prescaler 8 and Start Timer
    TCCR5B |= _BV(CS51); //set 1
  }
}

//spindle regulator
ISR(TIMER4_OVF_vect){
  //PI-Regulator
  //get Regulator-Parameter for 20KHz with 20 to 80% PWM and Ziegler-Nicols formula
  //#define K_P 1; //0,001 - 100 ???
  //#define KI_TN 1; //1s
  //T_Regler=20ms
/*
  int delta_revolution = STATE_RPM - target_revolutions;
  y = y_last + (((int32_t)K_P*(delta_revolution - delta_revolution_last))>>15) + (((int32_t)KI_TN * delta_revolution)>>15);
  OCR4C = map(y, REVOLUTIONS_MIN, REVOLUTIONS_MAX, OCR4C_min, OCR4C_max);
  y_last = y;
  delta_revolution_last = delta_revolution;
*/
}
