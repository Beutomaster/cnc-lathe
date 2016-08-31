#include "CNC_Lathe.h"

//global vars
boolean debug=true;

//Cosinus LookUp-Table for Quarter Circle in Q15 (max. 32767 !!!)
volatile const int lookup_cosinus[91] = {32767, 32762, 32747, 32722, 32687, 32642, 32587, 32523, 32448, 32364, 32269, 32165, 32051, 31927, 31794, 31650, 31498, 31335, 31163, 30982, 30791, 30591, 30381, 30162, 29934, 29697, 29451, 29196, 28932, 28659, 28377, 28087, 27788, 27481, 27165, 26841, 26509, 26169, 25821, 25465, 25101, 24730, 24351, 23964, 23571, 23170, 22762, 22347, 21925, 21497, 21062, 20621, 20173, 19720, 19260, 18794, 18323, 17846, 17364, 16876, 16384, 15886, 15383, 14876, 14364, 13848, 13328, 12803, 12275, 11743, 11207, 10668, 10126, 9580, 9032, 8481, 7927, 7371, 6813, 6252, 5690, 5126, 4560, 3993, 3425, 2856, 2286, 1715, 1144, 572, 0};

//ERROR-Numbers
volatile byte ERROR_NO = 0; //actual ERROR-Numbers Bit-coded (bit2_SPINDLE|bit1_CNC_CODE|bit0_SPI)

//Machine State
volatile byte STATE=0; //bit7_stepper|bit6_spindle_direction|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
volatile int STATE_RPM=0;
volatile int STATE_X=0;
volatile int STATE_Z=0;
volatile int STATE_F=0;
volatile int STATE_H=0;
volatile byte STATE_T=0; //0 = uninitialized
volatile int STATE_N=0;

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
  pinMode(PIN_DEBUG_INPUT_1, OUTPUT);
  pinMode(PIN_DEBUG_INPUT_2, OUTPUT);
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
  pinMode(PIN_USART1_RX, INPUT);
  pinMode(PIN_USART1_TX, OUTPUT);
  pinMode(PIN_SPI_MISO, OUTPUT); 		//Arduino is SPI-Slave
  pinMode(PIN_SPI_MOSI, INPUT); 	//Arduino is SPI-Slave
  pinMode(PIN_SPI_SCK, INPUT); 	//Arduino is SPI-Slave
  pinMode(PIN_SPI_SS, INPUT); 	//Arduino is SPI-Slave
  //potiservo.attach(PIN_SERVO_ENGINE);   //Attach Servo-Pin
  
  //Serial Communication
  Serial.begin(115200); //for Debugging with Serial Monitor
  Serial1.begin(9600); //Nikos Platine
  
  //SPI
  SPCR |= _BV(SPE);  // turn on SPI in slave mode
  create_machine_state_msg(); //initialize machine_state_msg before turning on interrupt
  SPI.attachInterrupt();  //turn on interrupt

  //Measurement of Revolutions
  attachInterrupt(digitalPinToInterrupt(PIN_REVOLUTIONS_SYNC),get_revolutions_ISR,RISING);

  //Observing old Control
  attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_OFF),get_stepper_on_off,CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_A),get_current_x_step,CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_X_B),get_current_x_step,CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_A),get_current_z_step,CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_OLD_CONTROL_STEPPER_Z_B),get_current_z_step,CHANGE);

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

  //set interrupt enable
  sei();

  //read Last Steps
  //read_last_x_step();
  //read_last_z_step();
}

void set_error(byte error_number) {
  ERROR_NO |= error_number;
}

void reset_error(byte error_number) {
  ERROR_NO &= ~error_number;
}

void loop() {
  // put your main code here, to run repeatedly:

  //SPI-Communication
  if (!byte_received && tx_buf[0]==100) create_machine_state_msg(); //update machine_state_msg if no transfer is in progress and no other message has to be sent
  spi_buffer_handling();

  //CNC-Lathe State-Machine  
  if (get_control_active()) {
    if (initialized) {
      if (!((STATE>>STATE_MANUAL_BIT)&1)) { //manual maybe not needed, instead use pause
        if (x_command_completed && z_command_completed) {
          command_completed=1;
          STATE_F = 0;
        }
        if (command_completed && !pause) {
          process_cnc_listing();
        }
        else stepper_timeout();
      }
      else stepper_timeout();
	  }
    //else intitialize(); //without sensors useless, Tool-Changer- and Origin-Init by SPI command (Origin not needed at the moment)
  }
  else {
    reset_initialization();
    observe_machine();
    set_revolutions(get_SERVO_CONTROL_POTI());
    //set spindle-direction
  }
}
