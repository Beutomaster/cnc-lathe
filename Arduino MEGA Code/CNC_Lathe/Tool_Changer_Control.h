#ifndef Tool_Changer_Control_h
#define Tool_Changer_Control_h

//includes
#include "CNC_Lathe.h"

//Positioning-Sensor or Input needed

extern volatile signed char i_tool;

void set_tool_position(byte);
void save_current_tool_position();
void read_current_tool_position();

#endif

