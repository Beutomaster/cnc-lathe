#ifndef PIN_Test_Spindle_h
#define PIN_Test_Spindle_h

#include <Arduino.h>
#include "Board_Tests.h"

//defines
#define REVOLUTIONS_MAX 3220 //rpm
#define REVOLUTIONS_MIN 460  //rpm
#define OCR5A_min 1088
#define OCR5A_max 4800
#define OCR4C_min 0 //value for 460 rpm ???
#define OCR4C_max 442

#define K_P 1 //0,001 - 100 ???
#define KI_TN 1 //1s

//Create new Servo Objekt
//extern Servo potiservo; //old Servo Lib

void spindle_on();
void spindle_off();
void set_revolutions(int);
int get_SERVO_CONTROL_POTI();
void set_poti_servo(int);
void set_spindle_new(boolean);

#endif

