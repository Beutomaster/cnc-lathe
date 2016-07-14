#ifndef Initialization_h
#define Initialization_h

//Freigabe oder Eingabe benoetigt.
//Kommunikation ueber Error-Code und Control-Signal

extern volatile char initialized;

void intitialize();
void initialize_tool_position(char);
void reset_initialization();

#endif

