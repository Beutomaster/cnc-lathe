#include "Tool_Changer_Control.h"

//Positionssensor oder Eingabe benoetigt

void set_tool_position(byte tool) {
  if ((tool>0) && (tool<7)) {
    //set new Tool Postion
    STATE_T=tool;
  
    if (initialized){
      //calc how many changes to make
      char i = tool - STATE_T;
      if (i<0) i = 6 + i;
      
      //set PINS with Timerinterrupt
      
      //set command duration
      command_running(TOOL_TIME * i);
    }
    else {
      //initialize
      STATE |= _BV(STATE_INIT_BIT); //set STATE_bit1 = STATE_INIT
      initialized=1;
    }
  }
}

byte get_tool_position() { //maybe not needed
	byte tool_position=0; //Stub
	return tool_position;
}

