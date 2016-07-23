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
      
      //Step1 TOOL_CHANGER_CHANGE 2,9s
      command_completed=0;
      digitalWrite(PIN_TOOL_CHANGER_HOLD, LOW);
      digitalWrite(PIN_TOOL_CHANGER_CHANGE, HIGH);

      /*
      //Step2 and 3 set PINS with Timerinterrupt
      //start Timer
      
      //Step2 TOOL_CHANGER_FIXING 3,5s
      digitalWrite(PIN_TOOL_CHANGER_CHANGE, LOW);
      digitalWrite(PIN_TOOL_CHANGER_FIXING, HIGH);
      
      //Step3 TOOL_CHANGER_HOLD
      digitalWrite(PIN_TOOL_CHANGER_FIXING, LOW);
      digitalWrite(PIN_TOOL_CHANGER_HOLD, HIGH);
      //stop Timer
      command_completed=1;
      */

      /* not needed anymore
      //set command duration
      command_running(TOOL_TIME * i);
      */
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

