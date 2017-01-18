#ifndef Initialization_h
#define Initialization_h

//includes
#include "CNC_Lathe.h"

//Freigabe oder Eingabe benoetigt.
//Kommunikation ueber Error-Code und Control-Signal

extern volatile boolean initialized;

//void intitialize();
//void initialize_tool_position(byte);
void reset_initialization();

#endif

