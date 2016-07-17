#include "Tool_Changer_Control.h"

//Positionssensor oder Eingabe benoetigt

void set_tool_position(char target_tool_position) {
  //calc how many changes to make
  char i = target_tool_position - STATE_T;
  if (i<0) i = 6 + i;
  
  //set PINS with Timerinterrupt

  //set new Tool Postion
  STATE_T=target_tool_position;
  
  //set command duration
  int command_time = TOOL_TIME * i;
  command_running(command_time);
}

char get_tool_position() { //maybe not needed
	char tool_position=0; //Stub
	return tool_position;
}

