#include "Tool_Changer_Control.h"

//Positionssensor oder Eingabe benoetigt

volatile byte tool_step=0;
volatile char i_tool=0;

void set_tool_position(byte tool) {
  if ((tool>0) && (tool<7)) {
      
    if (initialized){
      //calc how many changes to make
      i_tool = tool - STATE_T;
      if (i_tool<0) i_tool = 6 + i_tool;

      //if toolposition not reached yet
      if (i_tool>0) {
        //set new Tool Postion
        STATE_T=tool;

        if (debug && debug_tool) {
          Serial.print("Tool Changer starts moving to Position ");
          Serial.println(tool, DEC);
        }
      
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
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
          ICR1 = 45312; //ICR1 = T_ICF1*16MHz/Prescaler -1 = 2,9s*16MHz/1024 -1 = 45311,5 = 45312
          TCNT1 = 0; //set Start Value
        }
        TIFR1 = _BV(ICF1); //clear Interrupt flag by writing a logical one to it's bit, zeros don't alter the register
        //Input Compare Match Interrupt Enable
        TIMSK1 |= _BV(ICIE1); //set 1
        //Prescaler 1024 and Start Timer
        TCCR1B |= (_BV(CS12)|_BV(CS10)); //set 1
      }
    }
    else {
      //initialize
      STATE |= _BV(STATE_INIT_BIT); //set STATE_bit1 = STATE_INIT
      initialized=1;
    }
  }
}

void save_current_tool_position() { //instead of initialization, save in eeprom !!!
  EEPROM.write(LAST_TOOL_ADDRESS, STATE_T);
}

void read_current_tool_position() {
  STATE_T = EEPROM.read(LAST_TOOL_ADDRESS);
}

ISR(TIMER1_CAPT_vect) {
//Toolchanger-ISR
      if (tool_step==1) {
        //Step2 TOOL_CHANGER_FIXING 3,5s
        tool_step=2;
        digitalWrite(PIN_TOOL_CHANGER_CHANGE, LOW);
        //digitalWrite(PIN_TOOL_CHANGER_FIXING, HIGH);
        //set and start Timer1 for 3,5s
        ICR1 = 54687; //ICR1 = T_ICF1*16MHz/Prescaler -1 = 3,5s*16MHz/1024 -1 = 54686,5 = 54687
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
        //Input Compare Match Interrupt Disable
        TIMSK1 &= ~(_BV(ICIE1)); //set 0
        command_completed=1;
        }
        else {
          //Step1 TOOL_CHANGER_CHANGE 2,9s
          tool_step=1;
          digitalWrite(PIN_TOOL_CHANGER_HOLD, LOW);
          digitalWrite(PIN_TOOL_CHANGER_CHANGE, HIGH);
          ICR1 = 45312; //ICR1 = T_ICF1*16MHz/Prescaler -1 = 2,9s*16MHz/1024 -1 = 45311,5 = 45312
          TCNT1 = 0; //set Start Value
        }
      }
}

