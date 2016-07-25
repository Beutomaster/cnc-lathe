#include "Tool_Changer_Control.h"

//Positionssensor oder Eingabe benoetigt

volatile byte tool_step=0;

void set_tool_position(byte tool) {
  if ((tool>0) && (tool<7)) {
    //set new Tool Postion
    STATE_T=tool;
  
    if (initialized){
      //calc how many changes to make
      char i = tool - STATE_T;
      if (i<0) i = 6 + i;
      
      //Step1 TOOL_CHANGER_CHANGE 2,9s
      tool_step=1;
      command_completed=0;
      digitalWrite(PIN_TOOL_CHANGER_HOLD, LOW);
      digitalWrite(PIN_TOOL_CHANGER_CHANGE, HIGH);

      //Step2 and 3 set PINS with Timerinterrupt
      //set and start Timer1 for 2,9s
      TCCR1B = 0b00011000; //connect no Input-Compare-PINs, WGM13, WGM12 =1 for Fast PWM and Disbale Timer with Prescaler=0 while setting it up
      TCCR1A = 0b00000011; //connect no Output-Compare-PINs and WGM11, WGM10 =1 for Fast PWM
      TCCR1C = 0; //no Force of Output Compare
      OCR1A = 45313; //OCR1A = T_OCF1A*16MHz/Prescaler = 2,9s*16MHz/1024 = 45312,5 = 45313
      TCNT1 = 0; //set Start Value
      //Output Compare A Match Interrupt Enable
      TIMSK1 |= _BV(OCIE1A); //set 1
      //Prescaler 1024 and Start Timer
      TCCR1B |= (_BV(CS12)|_BV(CS10)); //set 1

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

ISR(TIMER1_COMPA_vect) {
//Toolchanger-ISR
      if (tool_step==1) {
        //Step2 TOOL_CHANGER_FIXING 3,5s
        tool_step=2;
        digitalWrite(PIN_TOOL_CHANGER_CHANGE, LOW);
        digitalWrite(PIN_TOOL_CHANGER_FIXING, HIGH);
        //set and start Timer1 for 3,5s
        OCR1A = 54688; //OCR1A = T_OCF1A*16MHz/Prescaler = 3,5s*16MHz/1024 = 54687,5 = 54688
        TCNT1 = 0; //set Start Value
      }

      if (tool_step==2) {
        //Step3 TOOL_CHANGER_HOLD
        digitalWrite(PIN_TOOL_CHANGER_FIXING, LOW);
        digitalWrite(PIN_TOOL_CHANGER_HOLD, HIGH);
        //stop Timer1
        //Output Compare A Match Interrupt Disable
        TIMSK1 &= ~(_BV(OCIE1A)); //set 0
        command_completed=1;
        tool_step=0;
      }
}

