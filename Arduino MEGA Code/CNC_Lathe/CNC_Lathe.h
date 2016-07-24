#ifndef CNC_Lathe_h
#define CNC_Lathe_h

//includes
#include <Arduino.h>
#include <EEPROM.h>
#include "CNC_Control.h"
#include "Control_Passiv.h"
#include "Initialization.h"
#include "Motion_Control.h"
#include "Raspi_SPI.h"
#include "Spindle_Control.h"
#include "Step_Motor_Control.h"
#include "Tool_Changer_Control.h"

//many global vars could be replaced by defines (no short names, because compiler reports no errors)
//many functions and vars should be private

//defines
//Bit Postions of STATE
#define STATE_CONTROL_ACTIVE_BIT 0
#define STATE_INIT_BIT 1
#define STATE_MANUAL_BIT 2
#define STATE_PAUSE_BIT 3
#define STATE_INCH_BIT 4
#define STATE_SPINDLE_BIT 5
#define STATE_STEPPER_BIT 6

//Bit Postions of ERROR_NO (actual ERROR-Numbers Bit-coded)
#define ERROR_SPI_BIT 0
#define ERROR_CNC_CODE_BIT 1
#define ERROR_SPINDLE_BIT 2

//PINs
#define PIN_CONTROL_INACTIVE 2       //Switch between EMCO and alternative Control (Usable for extINTR !!!)
#define PIN_SPINDELBOARD_NIKO 59       //Switch between Niko's Board and Hannes Servo
#define PIN_SPINDELPWM_NIKO 8           //PWM for Servo (Poti to set Revolutions)
#define PIN_SERVO_ENGINE 9           //PWM for Servo (Poti to set Revolutions)
#define PIN_STEPPER_X_A 10        //X35, PIN5 (A), Stepper X
#define PIN_STEPPER_X_B 11        //X35, PIN6 (B), Stepper X
#define PIN_STEPPER_X_C 12        //X35, PIN3 (C), Stepper X
#define PIN_STEPPER_X_D 13        //X35, PIN4 (D), Stepper X
#define PIN_STEPPER_Z_A 14        //X34, PIN5 (A), Stepper Z
#define PIN_STEPPER_Z_B 15        //X34, PIN6 (B), Stepper Z
#define PIN_STEPPER_Z_C 16        //X34, PIN3 (C), Stepper Z
#define PIN_STEPPER_Z_D 17        //X34, PIN4 (D), Stepper Z
#define PIN_TOOL_CHANGER_HOLD 4     //Tool-Changer hold (-3,3V)
#define PIN_TOOL_CHANGER_CHANGE 5     //Tool-Changer change (+12,9V)
#define PIN_TOOL_CHANGER_FIXING 6     //Tool-Changer fixing (-4,35V)
#define PIN_SPINDLE_ON 7          //Spindle on
#define PIN_REVOLUTIONS_SYNC 3     //Revolution-Sensor SYNC (Usable for extINTR !!!)
#define PIN_REVOLUTIONS_COUNT 62      //A08: Revolution-Sensor COUNT
#define PIN_OLD_CONTROL_STEPPER_X_A 63  //A09: X42, PIN5 (A), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_B 64  //A10: X42, PIN6 (B), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_C 65  //A11: X42, PIN3 (C), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_D 60  //A06: X42, PIN4 (D), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_A 66  //A12: X41, PIN5 (A), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_B 67  //A13: X41, PIN6 (B), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_C 68  //A12: X41, PIN3 (C), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_D 61  //A07: X41, PIN4 (D), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
//D68=A14 (IN): WZW Näherungssensor Position 0 ???
#define PIN_SPI_MISO 50         //D50 (OUT): SPI RaspBerry (Master) <-> Arduino (Slave)
#define PIN_SPI_MOSI 51         //D51 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
#define PIN_SPI_SCK 52          //D52 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
#define PIN_SPI_SS 53           //D53 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
//#define PIN_USART1_TX 18           //D18 (OUT) : Spindelplatine Niko: Drehzahlvorgabe per USART
//#define PIN_USART1_RX 19           //D19 (IN): Spindelplatine Niko: Drehzahlvorgabe per USART

//for use with analogRead(Analog-PIN-NR)
#define APIN_SERVO_CONTROL_POTI 15    //CONTROL-POTI to manually set revolutions (Analog-IN)

//EEPROM-Adresses
#define LAST_X_STEP_ADDRESS 0
#define LAST_Z_STEP_ADDRESS 1


//global vars
extern boolean debug;

//Cosinus LookUp-Table for Quarter Circle in Q15
extern volatile const int lookup_cosinus[91];

//ERROR-Numbers
extern volatile byte ERROR_NO;

//Machine State
extern volatile byte STATE; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
extern volatile int STATE_RPM;
extern volatile int STATE_X;
extern volatile int STATE_Z;
extern volatile int STATE_F;
extern volatile int STATE_H;
extern volatile byte STATE_T; //0 = uninitialized
extern volatile int STATE_N;

//functions
void set_error(byte);
void reset_error(byte);

#endif

