#ifndef Spindle_Control_h
#define Spindle_Control_h

//includes
#include "CNC_Lathe.h"
//#include <Servo.h>

//defines
#define OCR5A_min 870 //First Value which moves
#define OCR5A_max 5060 //280°
#define OCR4C_min 0 //value for 460 rpm ???
#define OCR4C_max 566
#define SPINDLE_ON_WAIT_TIME 500 //waiting time for savety
#define SPINDLE_OFF_WAIT_TIME 500 //waiting time for savety
#define RELAIS_WAIT_TIME 50 //waiting time for savety
#if defined BOARDVERSION_1_25 && defined SPINDLEDRIVER_NEW
  #define K_P 1 //0,001 - 100 ???
  #define KI_TN 1 //1s
#endif

//global ISR vars
extern volatile unsigned long rpm_time, last_rpm_time;
extern volatile int max_revolutions, target_revolutions;
extern volatile char wait_for_spindle_start, wait_for_spindle_stop, callback_spindle_direction_change, target_spindle_direction, wait_for_spindle_spindle_direction_relais, callback_spindle_start;
#if defined BOARDVERSION_1_25 && defined SPINDLEDRIVER_NEW
  extern volatile int delta_revolution_last;
  extern volatile long y, y_last;
  extern volatile boolean spindle_new;
#endif

#ifdef SERVO_BIB
  //Create new Servo Objekt
  extern Servo potiservo; //old Servo Lib
#endif

void spindle_on();
void spindle_off();
void spindle_direction(boolean);
void set_revolutions(int);
int get_SERVO_CONTROL_POTI();
void set_poti_servo(int);
void set_poti_servo_revolutions(int);
boolean test_for_spindle_rpm(int, int);
#if defined BOARDVERSION_1_25 && defined SPINDLEDRIVER_NEW
  void set_spindle_new(boolean);
#endif
void set_Timer5();
void get_revolutions_ISR();

#endif

