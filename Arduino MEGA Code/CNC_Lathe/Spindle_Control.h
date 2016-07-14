#ifndef Spindle_Control_h
#define Spindle_Control_h

#include <Servo.h>

#define REVOLUTIONS_MAX 3220 //rpm
#define REVOLUTIONS_MIN 460  //rpm

//Create new Servo Objekt
extern Servo potiservo;

void spindle(char);
void set_revolutions(int);
int get_revolutions();
int get_SERVO_CONTROL_POTI();
void set_poti_servo(int);

#endif

