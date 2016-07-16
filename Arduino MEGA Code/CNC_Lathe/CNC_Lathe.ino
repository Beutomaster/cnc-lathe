#include "CNC_Lathe.h"
#include "CNC_Control.h"
#include "Control_Passiv.h"
#include "Initialization.h"
#include "Motion_Control.h"
#include "Raspi_SPI.h"
#include "Spindle_Control.h"
#include "Step_Motor_Control.h"
#include "Tool_Changer_Control.h"

//PINs
const char PIN_CONTROL_INACTIVE = 2;       //Switch between EMCO and alternative Control
const char PIN_ROUTING_SWITCH = 3;        //Switch for Routing Signals to alternative Control
const char PIN_SERVO = 9;           //PWM for Servo (Poti to set Revolutions)
const char PIN_STEPPER_X_A = 10;        //X35, PIN5 (A), Stepper X
const char PIN_STEPPER_X_B = 11;        //X35, PIN6 (B), Stepper X
const char PIN_STEPPER_Z_A = 12;        //X34, PIN5 (A), Stepper Z
const char PIN_STEPPER_Z_B = 13;        //X34, PIN6 (B), Stepper Z
const char PIN_TOOL_CHANGER_HOLD = 4;     //Tool-Changer hold (-3,3V)
const char PIN_TOOL_CHANGER_CHANGE = 5;     //Tool-Changer change (+12,9V)
const char PIN_TOOL_CHANGER_FIXING = 6;     //Tool-Changer fixing (-4,35V)
const char PIN_SPINDLE_ON = 7;          //Spindle on
const char PIN_REVOLUTIONS_SYNC = 62;     //A8: Revolution-Sensor SYNC
const char PIN_REVOLUTIONS_COUNT = 63;      //A9: Revolution-Sensor COUNT
const char PIN_OLD_CONTROL_STEPPER_X_A = 64;  //A10: X42, PIN5 (A), Stepper X (Watching old Control)
const char PIN_OLD_CONTROL_STEPPER_X_B = 65;  //A11: X42, PIN6 (B), Stepper X (Watching old Control)
const char PIN_OLD_CONTROL_STEPPER_Z_A = 66;  //A12: X41, PIN5 (A), Stepper Z (Watching old Control)
const char PIN_OLD_CONTROL_STEPPER_Z_B = 67;  //A13: X41, PIN6 (B), Stepper Z (Watching old Control)
//D68=A14 (IN): WZW Näherungssensor Position 0 ???
const char PIN_SPI_MISO = 50;         //D50 (OUT): SPI RaspBerry (Master) <-> Arduino (Slave)
const char PIN_SPI_MOSI = 51;         //D51 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
const char PIN_SPI_SCK = 52;          //D52 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
const char PIN_SPI_SS = 53;           //D53 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
//const char PIN_USART1_TX = 18;           //D18 (OUT) : Spindelplatine Niko: Drehzahlvorgabe per USART
//const char PIN_USART1_RX = 19;           //D19 (IN): Spindelplatine Niko: Drehzahlvorgabe per USART

//for use with analogRead(Analog-PIN-NR)
const char APIN_SERVO_CONTROL_POTI = 15;    //CONTROL-POTI to manually set revolutions (Analog-IN)

//Stepper
int const xsteps_per_turn=72; //Steps per x-turn
int const zsteps_per_turn=72; //Steps per z-turn

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
  pinMode(PIN_SERVO, OUTPUT);
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
  potiservo.attach(PIN_SERVO);   //Attach Servo-Pin
  
  //Serial Communication
  Serial.begin(115200); //for Debugging with Serial Monitor
  Serial1.begin(9600); //Nikos Platine
  
  //SPI
  SPCR |= _BV(SPE);  // turn on SPI in slave mode
  create_machine_state_msg(); //initialize machine_state_msg before turning on interrupt
  SPI.attachInterrupt();  //turn on interrupt
}

void set_error(char error_number) {
  ERROR_NO |= error_number;
}

void reset_error(char error_number) {
  ERROR_NO &= ~error_number;
}


char process_control_signal() {
  char success=0;
  return success;
}

void loop() {
  // put your main code here, to run repeatedly:

  //SPI-Communication
  if (!byte_received && tx_buf[0]==100) create_machine_state_msg(); //update machine_state_msg if no transfer is in progress and no other message has to be sent
  spi_buffer_handling();

  //Control CNC-Lathe
  if (get_control_active()) {
    if (initialized) {
      if (!((STATE>>STATE_MANUAL_BIT)&1)) {
        process_cnc_listing();
      }
	  }
    //else intitialize(); //without sensors useless, Tool-Changer-init by SPI command
  }
  else {
    reset_initialization();
    observe_machine(); 
  }  
}
