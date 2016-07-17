#ifndef CNC_Lathe_h
#define CNC_Lathe_h

//includes
#include <Arduino.h>
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
#define PIN_CONTROL_INACTIVE 2       //Switch between EMCO and alternative Control
#define PIN_ROUTING_SWITCH 3        //Switch for Routing Signals to alternative Control
#define PIN_SERVO_ENGINE 9           //PWM for Servo (Poti to set Revolutions)
#define PIN_STEPPER_X_A 10        //X35, PIN5 (A), Stepper X
#define PIN_STEPPER_X_B 11        //X35, PIN6 (B), Stepper X
#define PIN_STEPPER_Z_A 12        //X34, PIN5 (A), Stepper Z
#define PIN_STEPPER_Z_B 13        //X34, PIN6 (B), Stepper Z
#define PIN_TOOL_CHANGER_HOLD 4     //Tool-Changer hold (-3,3V)
#define PIN_TOOL_CHANGER_CHANGE 5     //Tool-Changer change (+12,9V)
#define PIN_TOOL_CHANGER_FIXING 6     //Tool-Changer fixing (-4,35V)
#define PIN_SPINDLE_ON 7          //Spindle on
#define PIN_REVOLUTIONS_SYNC 62     //A8: Revolution-Sensor SYNC
#define PIN_REVOLUTIONS_COUNT 63      //A9: Revolution-Sensor COUNT
#define PIN_OLD_CONTROL_STEPPER_X_A 64  //A10: X42, PIN5 (A), Stepper X (Watching old Control)
#define PIN_OLD_CONTROL_STEPPER_X_B 65  //A11: X42, PIN6 (B), Stepper X (Watching old Control)
#define PIN_OLD_CONTROL_STEPPER_Z_A 66  //A12: X41, PIN5 (A), Stepper Z (Watching old Control)
#define PIN_OLD_CONTROL_STEPPER_Z_B 67  //A13: X41, PIN6 (B), Stepper Z (Watching old Control)
//D68=A14 (IN): WZW Näherungssensor Position 0 ???
#define PIN_SPI_MISO 50         //D50 (OUT): SPI RaspBerry (Master) <-> Arduino (Slave)
#define PIN_SPI_MOSI 51         //D51 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
#define PIN_SPI_SCK 52          //D52 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
#define PIN_SPI_SS 53           //D53 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave)
//#define PIN_USART1_TX 18           //D18 (OUT) : Spindelplatine Niko: Drehzahlvorgabe per USART
//#define PIN_USART1_RX 19           //D19 (IN): Spindelplatine Niko: Drehzahlvorgabe per USART

//for use with analogRead(Analog-PIN-NR)
#define APIN_SERVO_CONTROL_POTI 15    //CONTROL-POTI to manually set revolutions (Analog-IN)

//global vars
//ERROR-Numbers
extern unsigned char ERROR_NO;

//Machine State
extern unsigned char STATE; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
extern int STATE_RPM;
extern int STATE_X;
extern int STATE_Z;
extern int STATE_F;
extern int STATE_H;
extern char STATE_T; //0 = uninitialized
extern int STATE_N;

//functions
void set_error(unsigned char);
void reset_error(unsigned char);

#endif

