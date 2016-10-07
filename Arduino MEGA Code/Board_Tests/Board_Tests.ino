#include "Board_Tests.h"

int i=0, j=0;

char toolposition = 0;

volatile byte STATE_T=0;
volatile boolean command_completed=1;
volatile boolean debug_active=0, debug_rpm=0, debug_tool=1;

void setup() {
  // put your setup code here, to run once:
  //PINs
  pinMode(PIN_CONTROL_ACTIVE, INPUT);
  pinMode(PIN_REVOLUTIONS_SYNC, INPUT_PULLUP);
  pinMode(PIN_REVOLUTIONS_COUNT, INPUT_PULLUP);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_OFF, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_B, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_B, INPUT);
  pinMode(PIN_SERVO_ENGINE, OUTPUT); //needed for Fast PWM
  pinMode(PIN_SPINDELPWM_NIKO, OUTPUT); //needed for Fast PWM
  pinMode(PIN_SPINDLE_NEW, OUTPUT);
  pinMode(PIN_DEBUG_INPUT_1, INPUT);
  pinMode(PIN_DEBUG_INPUT_2, INPUT);
  pinMode(PIN_STEPPER_X_A, OUTPUT);
  pinMode(PIN_STEPPER_X_B, OUTPUT);
  pinMode(PIN_STEPPER_X_C, OUTPUT);
  pinMode(PIN_STEPPER_X_D, OUTPUT);
  pinMode(PIN_STEPPER_Z_A, OUTPUT);
  pinMode(PIN_STEPPER_Z_B, OUTPUT);
  pinMode(PIN_STEPPER_Z_C, OUTPUT);
  pinMode(PIN_STEPPER_Z_D, OUTPUT);
  pinMode(PIN_TOOL_CHANGER_HOLD, OUTPUT);
  pinMode(PIN_TOOL_CHANGER_CHANGE, OUTPUT);
  //pinMode(PIN_TOOL_CHANGER_FIXING, OUTPUT);
  pinMode(PIN_SPINDLE_ON, OUTPUT);
  pinMode(PIN_SPINDLE_DIRECTION, OUTPUT);
  //pinMode(PIN_USART1_RX, INPUT);
  //pinMode(PIN_USART1_TX, OUTPUT);
  //pinMode(PIN_SPI_MISO, OUTPUT); 		//Arduino is SPI-Slave
  //pinMode(PIN_SPI_MOSI, INPUT); 	//Arduino is SPI-Slave
  //pinMode(PIN_SPI_SCK, INPUT); 	//Arduino is SPI-Slave
  //pinMode(PIN_SPI_SS, INPUT); 	//Arduino is SPI-Slave
  //potiservo.attach(PIN_SERVO_ENGINE);   //Attach Servo-Pin
  
  //Serial Communication
  Serial.begin(115200); //for Debugging with Serial Monitor

  //TIMER
  
  //Timer0
  //millis() and micros() functions use Timer0
  //millis() returns (unsigned long)
  //micros() returned value at 16 MHz is always a multiple of four microseconds in (unsigned long)
  //get_revolutions and get_feed with micros() in PIN triggerd ISR !!! micros reads out Timer0 even in an ISR, but overflow could be missed
  //Dwell?
  //Stepper-Timeout

  //Timer1
  //Toolchanger + set command_completed
  //+X-Stepper
  //command_complete isr
  
  //Timer2 
  //tone() function uses Timer2
    
  //Timer3
  //Z-Stepper output + set command_completed while in active mode and maybe observing Stepper in passive mode

  //Timer4
  //spindle PWM
  //set and start Timer4 (Clk = 16MHz/(Prescaler*(TOP+1)) = 16MHz/(1023+1) = 15,625 kHz)
  TCCR4B = 0b00001000; //connect no Input-Compare-PINs, WGM43=0, WGM42=1 for Fast PWM, 10-bit and Disbale Timer with Prescaler=0 while setting it up
  TCCR4A = 0b00001011; //connect OC4C-PIN (PIN 8) to Output Compare and WGM41=1, WGM40=1 for Fast PWM with ICR4=TOP
  TCCR4C = 0; //no Force of Output Compare
  OCR4C = 0; //OCR4C max. = 1023 *0,55338792 = 566 !!! Engine is only for 180V DC
  TCNT4 = 0; //set Start Value
  //Prescaler 1 and Start Timer
  TCCR4B |= _BV(CS40); //set 1

  //Timer5 Servo and spindle regulator
  set_Timer5();

  //Spindle-Test
  //Quick and Dirty: Servo- and Spindle-PWM
  spindle_new = HIGH;
  digitalWrite(PIN_SPINDLE_NEW, HIGH);
  digitalWrite(PIN_SPINDLE_DIRECTION, HIGH);
  //set_spindle_new(HIGH); //Timer 5 for servo PWM, spindle just switched without regulator

  //set interrupt enable
  sei();

}

void loop() {
  // put your main code here, to run repeatedly:
  
  if (digitalRead(PIN_CONTROL_ACTIVE)) {
    if (debug_active) {
      //Debug
      Serial.println("Ardino active");
    }
    
    
    //Spindle-Test
    spindle_on();

    if (digitalRead(PIN_DEBUG_INPUT_1)) {
      //Stepper-Test
      //Debug
      Serial.println("Stepper-Test");
      //stepper_off();
      //delay(10);
      for (j=0; j<500; j++) {
          set_xstep(j%4);
          set_zstep(j%4);
          delay(5);
      }
      for (j=j-1; j>=0; j--) {
          set_xstep(j%4);
          set_zstep(j%4);
          delay(5);
      }
    }

    if (digitalRead(PIN_DEBUG_INPUT_2)) {
      if (command_completed) {
        //Tool-Changer-Test
        //Debug
        toolposition = random(1, 6);
        Serial.println("Tool-Changer-Test");
        Serial.print("Target Toolposition : ");
        Serial.println(toolposition, DEC);
        set_tool_position(toolposition);
      }
    }
    
  } else {
    if (debug_active) {
      //Debug
      Serial.println("Ardino inactive");
      spindle_off();
    }
  }
  
  set_revolutions(get_SERVO_CONTROL_POTI());
}
