#ifndef Step_Motor_Control_h
#define Step_Motor_Control_h

#include <Stepper.h>

extern Stepper xstepper; //X-Motor-Object
extern Stepper zstepper; //Z-Motor-Object

void x_stepper(char, char);
void z_stepper(char, char);
void set_x_steps(int, int);
void set_z_steps(int, int);
int count_x_steps();
int count_z_steps();
char get_current_x_step();
char get_current_z_step();

#endif

