#include "CNC_Lathe.h"

//global vars
//ERROR-Numbers
unsigned char ERROR_NO = 0; //actual ERROR-Numbers Bit-coded (bit2_SPINDLE|bit1_CNC_CODE|bit0_SPI)

//Machine State
unsigned char STATE=0; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
int STATE_RPM=0;
int STATE_X=0;
int STATE_Z=0;
int STATE_F=0;
int STATE_H=0;
char STATE_T=0; //0 = uninitialized
int STATE_N=0;

void setup() {
  // put your setup code here, to run once:
  //PINs
  pinMode(PIN_CONTROL_INACTIVE, INPUT_PULLUP); //LOW-Active (GND = Control activ)
  pinMode(PIN_REVOLUTIONS_SYNC, INPUT);
  pinMode(PIN_REVOLUTIONS_COUNT, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_B, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_B, INPUT);
  pinMode(PIN_ROUTING_SWITCH, OUTPUT);
  pinMode(PIN_SERVO_ENGINE, OUTPUT);
  pinMode(PIN_STEPPER_X_A, OUTPUT);
  pinMode(PIN_STEPPER_X_B, OUTPUT);
  pinMode(PIN_STEPPER_Z_A, OUTPUT);
  pinMode(PIN_STEPPER_Z_B, OUTPUT);
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
}

void set_error(unsigned char error_number) {
  ERROR_NO |= error_number;
}

void reset_error(unsigned char error_number) {
  ERROR_NO &= ~error_number;
}

void loop() {
  // put your main code here, to run repeatedly:

  //SPI-Communication
  if (!byte_received && tx_buf[0]==100) create_machine_state_msg(); //update machine_state_msg if no transfer is in progress and no other message has to be sent
  spi_buffer_handling();

  //CNC-Lathe State-Machine
  observe_machine();
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
    set_revolutions(get_SERVO_CONTROL_POTI());
  }
}
