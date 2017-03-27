#ifndef Spindle_Control_h
#define Spindle_Control_h

//includes
#include "CNC_Lathe.h"
#ifdef SERVO_LIB
  #include <Servo.h>
#endif

//defines
#define OCR5A_min 870 //First Value which moves
#define OCR5A_max 5060 //280°
#define OCR4C_min 0 //value for 460 rpm ???
#define OCR4C_max 566
#ifndef SPINDLE_STATE_CODE_NEW
  #define SPINDLE_ON_WAIT_TIME 500 //waiting time for savety
  #define SPINDLE_OFF_WAIT_TIME 500 //waiting time for savety
  #define RELAIS_WAIT_TIME 50 //waiting time for savety
#else
  #define SPINDLE_CHARGE_RESISTOR_WAIT_TIME 5000 //in ms
  #define SPINDLE_START_WAIT_TIME 5000 //in ms
  #define SPINDLE_STOP_WAIT_TIME 5000 //in ms
  #define RELAIS_WAIT_TIME 500 //in ms
#endif
#if defined BOARDVERSION_1_25 && defined SPINDLEDRIVER_NEW
  #define K_P 1 //0,001 - 100 ???
  #define KI_TN 1 //1s
#endif

//SPINDLE_STATES
#define SPINDLE_STATE_CHARGE_RESISTOR_ON 0 //only needed with SPINDLEDRIVER_NEW
#define SPINDLE_STATE_SPINDLE_OFF 1
#define SPINDLE_STATE_DIRECTION_CHANGE 2
#define SPINDLE_STATE_SPINDLE_START 3
#define SPINDLE_STATE_SPINDLE_ON 4
#define SPINDLE_STATE_SPINDLE_STOP 5
#define SPINDLE_STATE_SPINDLE_ERROR 6

//global ISR vars
extern volatile unsigned long rpm_time, last_rpm_time;
extern volatile int max_revolutions, target_revolutions;
extern volatile char wait_for_spindle_start, wait_for_spindle_stop, callback_spindle_direction_change, target_spindle_on, target_spindle_direction, wait_for_spindle_spindle_direction_relais, callback_spindle_start;
#if defined BOARDVERSION_1_25 && defined SPINDLEDRIVER_NEW
  extern volatile int delta_revolution_last;
  extern volatile long y, y_last;
  extern volatile boolean spindle_new;
#endif
extern volatile int adcvalue; // adc value read by ISR(ADC_vect)
#ifdef SPINDLE_STATE_CODE_NEW
  extern volatile unsigned int rpm_count;
#endif

//other global vars
extern char spindle_state, spindle_command_completed;
#ifdef SPINDLE_STATE_CODE_NEW
  extern unsigned long spindle_wait_timestamp;
#endif

#ifdef SERVO_LIB
  //Create new Servo Objekt
  extern Servo potiservo; //old Servo Lib
#endif

void spindle_on();
void spindle_off();
void spindle_direction(boolean);
#ifdef SPINDLE_STATE_CODE_NEW
  void spindle_direction_private(boolean);
#endif
boolean test_for_spindle_off();
void get_spindle_state_passiv();
void set_spindle_state_spindle_on();
void set_spindle_state_spindle_off();
void set_spindle_state_spindle_error();
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
void intr_analogRead(uint8_t);

#endif

