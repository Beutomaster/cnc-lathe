#ifndef Control_Passiv_h
#define Control_Passiv_h

//includes
#include "CNC_Lathe.h"

boolean get_control_active();
void observe_machine();
void enable_observe_machine_PCINT2();
void disable_observe_machine_PCINT2();

#endif

