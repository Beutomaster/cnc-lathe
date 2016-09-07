#include "PIN_Test_Tool_Changer.h"

//Positionssensor oder Eingabe benoetigt

volatile byte tool_step=0;
volatile char i_tool=0;

void set_tool_position(byte tool) {
  if ((tool>0) && (tool<7)) {
        
    //calc how many changes to make
    char i_tool = tool - STATE_T;
    if (i_tool<0) i_tool = 6 + i_tool;

    //set new Tool Postion
    STATE_T=tool;
    
    //Step1 TOOL_CHANGER_CHANGE 2,9s
    tool_step=1;
    command_completed=0;
    digitalWrite(PIN_TOOL_CHANGER_HOLD, LOW);
    digitalWrite(PIN_TOOL_CHANGER_CHANGE, HIGH);

    //Step1 and 2 are setting PINS in Timerinterrupt
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
  }
}


ISR(TIMER1_COMPA_vect) {
//Toolchanger-ISR
      if (tool_step==1) {
        //Step2 TOOL_CHANGER_FIXING 3,5s
        tool_step=2;
        digitalWrite(PIN_TOOL_CHANGER_CHANGE, LOW);
        //digitalWrite(PIN_TOOL_CHANGER_FIXING, HIGH);
        //set and start Timer1 for 3,5s
        OCR1A = 54688; //OCR1A = T_OCF1A*16MHz/Prescaler = 3,5s*16MHz/1024 = 54687,5 = 54688
        TCNT1 = 0; //set Start Value
      }

      else if (tool_step==2) {
        //Step0 TOOL_CHANGER_HOLD
        tool_step=0;
        //digitalWrite(PIN_TOOL_CHANGER_FIXING, LOW);
        digitalWrite(PIN_TOOL_CHANGER_HOLD, HIGH);
        i_tool--;
        if (i_tool==0) {
        //stop Timer1
        //Output Compare A Match Interrupt Disable
        TIMSK1 &= ~(_BV(OCIE1A)); //set 0
        command_completed=1;
        }
        else {
          //Step1 TOOL_CHANGER_CHANGE 2,9s
          tool_step=1;
          digitalWrite(PIN_TOOL_CHANGER_HOLD, LOW);
          digitalWrite(PIN_TOOL_CHANGER_CHANGE, HIGH);
          OCR1A = 45313; //OCR1A = T_OCF1A*16MHz/Prescaler = 2,9s*16MHz/1024 = 45312,5 = 45313
          TCNT1 = 0; //set Start Value
        }
      }
}

