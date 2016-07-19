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

void set_tool_position(char);
char get_tool_position();

#endif

