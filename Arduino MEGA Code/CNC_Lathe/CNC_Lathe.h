#ifndef CNC_Lathe_h
#define CNC_Lathe_h

//many global vars could be replaced by defines (no short names, because compiler reports no errors)
//some chars should be boolean

//Bit Postions of STATE
#define STATE_CONTROL_ACTIVE_BIT 0
#define STATE_INIT_BIT 1
#define STATE_MANUAL_BIT 2
#define STATE_INCH_BIT 3
#define STATE_SPINDLE_BIT 4
#define STATE_STEPPER_BIT 5

//Bit Postions of ERROR_NO (actual ERROR-Numbers Bit-coded)
#define ERROR_SPI_BIT 0;
#define ERROR_CNC_CODE_BIT 1;
#define ERROR_SPINDLE_BIT 2;

//PINs
extern const char PIN_CONTROL_INACTIVE; 			//Switch between EMCO and alternative Control
extern const char PIN_ROUTING_SWITCH; 				//Switch for Routing Signals to alternative Control
extern const char PIN_SERVO;						//PWM for Servo (Poti to set Revolutions)
extern const char PIN_STEPPER_X_A;				//X35, PIN5 (A), Stepper X
extern const char PIN_STEPPER_X_B;				//X35, PIN6 (B), Stepper X
extern const char PIN_STEPPER_Z_A;				//X34, PIN5 (A), Stepper Z
extern const char PIN_STEPPER_Z_B;				//X34, PIN6 (B), Stepper Z
extern const char PIN_TOOL_CHANGER_HOLD;			//Tool-Changer hold (-3,3V)
extern const char PIN_TOOL_CHANGER_CHANGE;			//Tool-Changer change (+12,9V)
extern const char PIN_TOOL_CHANGER_FIXING;			//Tool-Changer fixing (-4,35V)
extern const char PIN_SPINDLE_ON;					//Spindle on
extern const char PIN_REVOLUTIONS_SYNC;			//A8: Revolution-Sensor SYNC
extern const char PIN_REVOLUTIONS_COUNT;			//A9: Revolution-Sensor COUNT
extern const char PIN_OLD_CONTROL_STEPPER_X_A;	//A10: X42, PIN5 (A), Stepper X (Watching old Control)
extern const char PIN_OLD_CONTROL_STEPPER_X_B;	//A11: X42, PIN6 (B), Stepper X (Watching old Control)
extern const char PIN_OLD_CONTROL_STEPPER_Z_A;	//A12: X41, PIN5 (A), Stepper Z (Watching old Control)
extern const char PIN_OLD_CONTROL_STEPPER_Z_B;	//A13: X41, PIN6 (B), Stepper Z (Watching old Control)
//D68=A14 (IN):	WZW Näherungssensor Position 0 ???
extern const char PIN_SPI_MISO;					//D50 (OUT): SPI RaspBerry (Master) <-> Arduino (Slave)
extern const char PIN_SPI_MOSI;					//D51 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
extern const char PIN_SPI_SCK;					//D52 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
extern const char PIN_SPI_SS;						//D53 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
extern const char PIN_USART0_RX; 					//D0 (OUT): Spindelplatine Niko: Drehzahlvorgabe per USART
extern const char PIN_USART0_TX; 					//D1 (IN) : Spindelplatine Niko: Drehzahlvorgabe per USART

//for use with analogRead(Analog-PIN-NR)
extern const char APIN_SERVO_CONTROL_POTI;		//CONTROL-POTI to manually set revolutions (Analog-IN)

//Stepper
extern const int xsteps_per_turn; //Steps per x-turn
extern const int zsteps_per_turn; //Steps per z-turn

//ERROR-Numbers
extern unsigned char ERROR_NO;

//Machine State
extern unsigned char STATE; //bit5_stepper|bit4_spindle|bit3_inch|bit2_manual|bit1_init|bit0_control_active
extern int STATE_RPM;
extern int STATE_X;
extern int STATE_Z;
extern int STATE_F;
extern int STATE_H;
extern char STATE_T; //0 = uninitialized
extern int STATE_N;

//functions
void set_error(char);
void reset_error(char);
char process_control_signal();

#endif

