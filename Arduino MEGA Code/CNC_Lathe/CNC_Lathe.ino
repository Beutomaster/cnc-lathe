#include "CNC_Lathe.h"

//global vars

//Cosinus LookUp-Table for Quarter Circle in Q15
volatile const int lookup_cosinus[91] = {32768, 32763, 32748, 32723, 32688, 32643, 32588, 32524, 32449, 32365, 32270, 32166, 32052, 31928, 31795, 31651, 31499, 31336, 31164, 30983, 30792, 30592, 30382, 30163, 29935, 29698, 29452, 29197, 28932, 28660, 28378, 28088, 27789, 27482, 27166, 26842, 26510, 26170, 25822, 25466, 25102, 24730, 24351, 23965, 23571, 23170, 22763, 22348, 21926, 21498, 21063, 20622, 20174, 19720, 19261, 18795, 18324, 17847, 17364, 16877, 16384, 15886, 15384, 14876, 14365, 13848, 13328, 12803, 12275, 11743, 11207, 10668, 10126, 9580, 9032, 8481, 7927, 7371, 6813, 6252, 5690, 5126, 4560, 3993, 3425, 2856, 2286, 1715, 1144, 572, 0};

//ERROR-Numbers
byte ERROR_NO = 0; //actual ERROR-Numbers Bit-coded (bit2_SPINDLE|bit1_CNC_CODE|bit0_SPI)

//Machine State
byte STATE=0; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
int STATE_RPM=0;
int STATE_X=0;
int STATE_Z=0;
int STATE_F=0;
int STATE_H=0;
byte STATE_T=0; //0 = uninitialized
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

  //TIMER
  //get_revolutions, get_feed ???
  
  //Timer0, millis(), micros()
  //Stepper-Timeout
  
  //Timer2
  //Toolchanger command_complete
  //TCCR2B = 0x00;        //Disbale Timer2 while we set it up
  //ASSR &= ~(_BV(AS2)); //set 0 to Select clk_T2S=CLK_IO
  //Prescaler 256
  //TCCR2B &= ~(_BV(CS20)); //set 0
  //TCCR2B |= _BV(CS21); //set 1
  //TCCR2B |= _BV(CS22); //set 1
  //Normal Mode
  //TCCR2A &= ~(_BV(WGM20)); //set 0
  //TCCR2A &= ~(_BV(WGM21)); //set 0
  //TCCR2B &= ~(_BV(WGM22)); //set 0
  //Timer Overflows INTR enable
  //TIMSK2 |= _BV(TOIE2); //set 1
  //cli() //global INTR enable

  
  //Timer3,4
  //2x Stepper active and passsive, command_complete

  //Timer5 Servo
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
