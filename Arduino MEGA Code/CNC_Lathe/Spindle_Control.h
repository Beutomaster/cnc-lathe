#ifndef Spindle_Control_h
#define Spindle_Control_h

//includes
#include <Arduino.h>
//#include <Servo.h>
#include "CNC_Lathe.h"

//defines
#define REVOLUTIONS_MAX 3220 //rpm
#define REVOLUTIONS_MIN 460  //rpm
#define OCR5A_min 1088
#define OCR5A_max 4800
#define OCR4C_max 442

//Create new Servo Objekt
//extern Servo potiservo; //old Servo Lib

void spindle_on();
void spindle_off();
void set_revolutions(int);
int get_SERVO_CONTROL_POTI();
void set_poti_servo(int);
void get_revolutions_ISR();

#endif

