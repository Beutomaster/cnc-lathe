#ifndef Control_Passiv_h
#define Control_Passiv_h

//includes
#include <util/atomic.h>
#include <Arduino.h>
#include "CNC_Lathe.h"

extern boolean control_active;

void get_control_active();
void observe_machine();

#endif

