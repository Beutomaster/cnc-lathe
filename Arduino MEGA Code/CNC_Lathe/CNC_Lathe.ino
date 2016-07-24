#include "CNC_Lathe.h"

//global vars
boolean debug=true;

//Cosinus LookUp-Table for Quarter Circle in Q15 (max. 32767 !!!)
volatile const int lookup_cosinus[91] = {32768, 32763, 32748, 32723, 32688, 32643, 32588, 32524, 32449, 32365, 32270, 32166, 32052, 31928, 31795, 31651, 31499, 31336, 31164, 30983, 30792, 30592, 30382, 30163, 29935, 29698, 29452, 29197, 28932, 28660, 28378, 28088, 27789, 27482, 27166, 26842, 26510, 26170, 25822, 25466, 25102, 24730, 24351, 23965, 23571, 23170, 22763, 22348, 21926, 21498, 21063, 20622, 20174, 19720, 19261, 18795, 18324, 17847, 17364, 16877, 16384, 15886, 15384, 14876, 14365, 13848, 13328, 12803, 12275, 11743, 11207, 10668, 10126, 9580, 9032, 8481, 7927, 7371, 6813, 6252, 5690, 5126, 4560, 3993, 3425, 2856, 2286, 1715, 1144, 572, 0};

//ERROR-Numbers
volatile byte ERROR_NO = 0; //actual ERROR-Numbers Bit-coded (bit2_SPINDLE|bit1_CNC_CODE|bit0_SPI)

//Machine State
volatile byte STATE=0; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
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
  pinMode(PIN_CONTROL_INACTIVE, INPUT_PULLUP); //LOW-Active (GND = Control activ)
  pinMode(PIN_REVOLUTIONS_SYNC, INPUT);
  pinMode(PIN_REVOLUTIONS_COUNT, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_B, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_C, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_D, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_B, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_C, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_D, INPUT);
  pinMode(PIN_SERVO_ENGINE, OUTPUT);
  pinMode(PIN_SPINDELBOARD_NIKO, OUTPUT);
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
  pinMode(PIN_TOOL_CHANGER_FIXING, OUTPUT);
  pinMode(PIN_SPINDLE_ON, OUTPUT);
  //pinMode(PIN_USART1_RX, INPUT);
  //pinMode(PIN_USART1_TX, OUTPUT);
  pinMode(PIN_SPI_MISO, OUTPUT); 		//Arduino is SPI-Slave
  pinMode(PIN_SPI_MOSI, INPUT); 	//Arduino is SPI-Slave
  pinMode(PIN_SPI_SCK, INPUT); 	//Arduino is SPI-Slave
  pinMode(PIN_SPI_SS, INPUT); 	//Arduino is SPI-Slave
  potiservo.attach(PIN_SERVO_ENGINE);   //Attach Servo-Pin
  
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
  //Problem: Switching Stepper off in Step 0 can't be detected
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
  //Stepper-Timeout

  //Timer1
  //Toolchanger + command_complete
  TCCR1B = 0; //Disbale Timer while we set it up and Normal Mode
  TCCR1A = 0; //Normal Mode
  TCCR1C = 0; //Normal Mode
  //Timer Overflow INTR enable
  TIMSK1 |= _BV(TOIE1); //set 1
  //Prescaler 1 and Start Timer => Overflow with 16MHz/((2^16)*Prescaler) = 244,14Hz/Prescaler => we need a different timer operation mode or interrupt !!!
  TCCR1B |= _BV(CS10); //set 1
  
  //Timer2 
  //tone() function uses Timer2
    
  //Timer3
  //2x Stepper output + command_complete while in active mode and maybe observing Stepper in passsive mode 
  TCCR3B = 0; //Disbale Timer while we set it up and Normal Mode
  TCCR3A = 0; //Normal Mode
  TCCR3C = 0; //Normal Mode
  //Timer Overflow INTR enable
  TIMSK3 |= _BV(TOIE3); //set 1
  //Prescaler 1 and Start Timer => Overflow with 16MHz/((2^16)*Prescaler) = 244,14Hz/Prescaler => we need a different timer operation mode or interrupt !!!
  TCCR3B |= _BV(CS30); //set 1

  //Timer4
  //Niko's spindle regulator
  
  //Timer5 Servo

  //cli() //global INTR enable
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
  }
}
