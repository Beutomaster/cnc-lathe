#ifndef Tool_Changer_Control_h
#define Tool_Changer_Control_h

//includes
#include <Arduino.h>
#include <util/atomic.h>
#include "CNC_Lathe.h"
#include "CNC_Control.h"
#include "Motion_Control.h"

//Positionssensor oder Eingabe benoetigt

void set_tool_position(byte);
void save_current_tool_position();
void read_current_tool_position();

#endif

