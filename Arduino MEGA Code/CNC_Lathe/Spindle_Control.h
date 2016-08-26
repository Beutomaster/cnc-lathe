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
#define OCR4C_min 0 //value for 460 rpm ???
#define OCR4C_max 566

#define K_P 1 //0,001 - 100 ???
#define KI_TN 1 //1s

extern volatile bool spindle_new;

//Create new Servo Objekt
//extern Servo potiservo; //old Servo Lib

void spindle_on();
void spindle_off();
void spindle_direction(bool);
void set_revolutions(int);
int get_SERVO_CONTROL_POTI();
void set_poti_servo(int);
void set_spindle_new(bool);
void set_Timer5();
void get_revolutions_ISR();

#endif

