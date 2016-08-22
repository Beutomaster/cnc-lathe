#ifndef Tool_Changer_Control_h
#define Tool_Changer_Control_h

//includes
#include <Arduino.h>
#include "CNC_Lathe.h"
#include "CNC_Control.h"
#include "Motion_Control.h"

//defines
#define TOOL_TIME 7 //needs to be set for 6,4s !!!

//Positionssensor oder Eingabe benoetigt

void set_tool_position(byte);
byte get_tool_position();
void save_current_tool_position();
void read_current_tool_position();

#endif

