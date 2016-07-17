#include "Spindle_Control.h"

//Create new Servo Objekt
Servo potiservo;

void spindle_on() {
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
  int rev_niko=map(target_revolutions, 0, REVOLUTIONS_MAX, 0, 255);
  Serial.println (rev_niko); //for debugging
  Serial1.println (rev_niko);
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
	potiservo.write(poti_angle);
}

void get_revolutions_ISR() { //read revolution-sensor
  //has to be an Timer/Compare-ISR
  STATE_RPM = 0; //stub
}
