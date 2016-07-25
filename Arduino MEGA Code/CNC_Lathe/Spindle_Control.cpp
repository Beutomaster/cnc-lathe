#include "Spindle_Control.h"

//global ISR vars
volatile unsigned long rpm_time=0, last_rpm_time=0;
volatile long clk_xfeed=0, clk_zfeed=0;

//Create new Servo Objekt
//Servo potiservo;  //old Servo Lib

void spindle_on() {
  last_rpm_time = micros();
  digitalWrite(PIN_SPINDLE_ON, HIGH);
  STATE |= _BV(STATE_SPINDLE_BIT); //set STATE_bit5 = spindle
}

void spindle_off() {
  digitalWrite(PIN_SPINDLE_ON, LOW);
  STATE &= ~(_BV(STATE_SPINDLE_BIT)); //delete STATE_bit5 = spindle
}

void set_revolutions(int target_revolutions) {
	//Poti-Servo
	int poti_angle = map(target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, 0, 180);
	set_poti_servo(poti_angle);
 
	//Alternativ über serielle Schnittstelle an Niko's Platine (0 to 255)
  byte rev_niko=map(target_revolutions, 0, REVOLUTIONS_MAX, 0, 255);
  if (debug) { //for debugging
    Serial.print("RPM-set-Value Niko: ");
    Serial.println (rev_niko);
  }
  Serial1.write (rev_niko);
  
  //Timer4 Fast PWM (OC4C) for Niko's spindle driver (set Revolutions)
  //min. 16KHz?
  //230V AC * sqrt(2) => ca. 325 V DC - deltaU/2 (Glättung)
  //Motor max. 180 V DC * 100 /325 V DC = 55,384615384615384615384615384615 %
  OCR4C = map(target_revolutions, REVOLUTIONS_MIN, REVOLUTIONS_MAX, 0, OCR4C_max);
  TCNT4 = 0; //set Start Value
}

int get_SERVO_CONTROL_POTI() {
	//Reading Poti-Value
	int manual_target_revolutions = analogRead(APIN_SERVO_CONTROL_POTI);
	
	//Convert 10-bit Value from Analog-Input (0-1023) to 460-3220 rpm
	manual_target_revolutions = map(manual_target_revolutions, 0, 1023, REVOLUTIONS_MIN, REVOLUTIONS_MAX);

	return manual_target_revolutions;
}

void set_poti_servo(int poti_angle){
	//write angle in degree to Servo-Objekt
	//potiservo.write(poti_angle); //old Servo Lib
  OCR5A = (OCR5A_max-OCR5A_min)*poti_angle/180 + OCR5A_min; //OCR5A = T_OCF5A*16MHz/Prescaler = 544µs*16MHz/8 = 1088 ... OCR5A = 2400µs*16MHz/8 = 4800
}

void get_revolutions_ISR() { //read revolution-sensor
  //first value may be wrong at passive modus, overflow of timer0 may cause errors
  rpm_time = micros();
  STATE_RPM = 600000L/(rpm_time-last_rpm_time); //(60s/min)*(1000ms/s)*(1000us/ms)/(100sync/U) = 600000
  last_rpm_time = rpm_time;
}

//spindle regulator
ISR(TIMER4_OVF_vect){
  //Regulator
  //OCR4C = 0;
}

