#ifndef Spindle_Control_h
#define Spindle_Control_h

//includes
#include <Arduino.h>
//#include <Servo.h>
#include "CNC_Lathe.h"

//defines
#define REVOLUTIONS_MAX 3220 //rpm
#define REVOLUTIONS_MIN 460  //rpm
#define OCR5A_min 1091
#define OCR5A_max 4799
#define OCR4C_min 0 //value for 460 rpm ???
#define OCR4C_max 566

#define K_P 1 //0,001 - 100 ???
#define KI_TN 1 //1s

//global ISR vars
extern volatile unsigned long rpm_time, last_rpm_time;
extern volatile long y, y_last;
extern volatile int target_revolutions, delta_revolution_last, max_revolutions;
extern volatile boolean spindle_new;

//Create new Servo Objekt
//extern Servo potiservo; //old Servo Lib

void spindle_on();
void spindle_off();
void spindle_direction(boolean);
void set_revolutions(int);
int get_SERVO_CONTROL_POTI();
void set_poti_servo(int);
void set_spindle_new(boolean);
void set_Timer5();
void get_revolutions_ISR();

#endif

