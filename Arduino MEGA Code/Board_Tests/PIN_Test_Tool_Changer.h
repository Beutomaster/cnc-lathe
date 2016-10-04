#ifndef PIN_Test_Tool_Changer_h
#define PIN_Test_Tool_Changer_h

//includes
#include <Arduino.h>
#include "Board_Tests.h"

//defines
#define TOOL_TIME 7 //needs to be set for 6,4s !!!

//maybe needed for isr
extern volatile byte tool_step;
extern volatile char i_tool;

//Positionssensor oder Eingabe benoetigt

void set_tool_position(byte);

#endif

