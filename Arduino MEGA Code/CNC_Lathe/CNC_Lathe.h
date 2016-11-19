#ifndef CNC_Lathe_h
#define CNC_Lathe_h

//includes
#include <util/atomic.h>
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
#define STATE_SPINDLE_DIRECTION_BIT 6
#define STATE_STEPPER_BIT 7

//Bit Postions of ERROR_NO (actual ERROR-Numbers Bit-coded)
#define ERROR_SPI_BIT 0
#define ERROR_CNC_CODE_BIT 1
#define ERROR_SPINDLE_BIT 2

//PINs
#define PIN_CONTROL_ACTIVE 2       //PE4: Switch between EMCO and alternative Control (Usable for extINTR !!!)
#define PIN_REVOLUTIONS_SYNC 3     //PE5: Revolution-Sensor SYNC (Usable for extINTR !!!) We should change the PIN with PIN_REVOLUTIONS_COUNT in V2!!!
#define PIN_TOOL_CHANGER_HOLD 4     //PG5: Tool-Changer hold (-3,3V)
#define PIN_TOOL_CHANGER_CHANGE 5     //PE3: Tool-Changer change (+12,9V)
//#define PIN_TOOL_CHANGER_FIXING 6     //PH3: Tool-Changer fixing (-4,35V)
#define PIN_SPINDLE_ON 7          //PH4: Spindle on
#define PIN_SPINDELPWM_NIKO 8           //PH5: Timer4 Fast PWM (OC4C) for Niko's spindle driver (set Revolutions)
#define PIN_SPINDLE_DIRECTION 9         //PH6: HIGH=Inverse Direction
#define PIN_STEPPER_X_A 10        //PB4: X35, PIN5 (A), Stepper X
#define PIN_STEPPER_X_B 11        //PB5: X35, PIN6 (B), Stepper X
#define PIN_STEPPER_X_C 12        //PB6: X35, PIN3 (C), Stepper X
#define PIN_STEPPER_X_D 13        //PB7: X35, PIN4 (D), Stepper X
#define PIN_STEPPER_Z_A 14        //PJ1: X34, PIN5 (A), Stepper Z
#define PIN_STEPPER_Z_B 15        //PJ0: X34, PIN6 (B), Stepper Z
#define PIN_STEPPER_Z_C 17 //Hotfix, planed was 16        //PH0: PH1: X34, PIN3 (C), Stepper Z
#define PIN_STEPPER_Z_D 16 //Hotfix, planed was 17        //X34, PIN4 (D), Stepper Z
#define PIN_USART1_TX 18           //D18 (OUT) : Spindelplatine Niko: Drehzahlvorgabe per USART
#define PIN_USART1_RX 19           //D19 (IN): Spindelplatine Niko: Drehzahlvorgabe per USART
#define PIN_SERVO_ENGINE 46         //PL3: PWM for Servo (Poti to set Revolutions) Timer5 PWM on Pins 44,45,46 is occupied by Servo.h (but it is not necessary to choose these pins)
//#define PIN_SPI_MISO 50         //D50 (OUT): SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
//#define PIN_SPI_MOSI 51         //D51 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
//#define PIN_SPI_SCK 52          //D52 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
//#define PIN_SPI_SS 53           //D53 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
#define PIN_SPINDLE_NEW 59    //PF5: Switch between Niko's Board and Hannes Servo
#define PIN_DEBUG_INPUT_1 60    //Debug Input 1
#define PIN_DEBUG_INPUT_2 61    //Debug Input 2
#define PIN_REVOLUTIONS_COUNT 62    //PK0, A08: Revolution-Sensor COUNT
#define PIN_OLD_CONTROL_STEPPER_X_OFF 65 //PK3, A09: X42, PIN3,4,5,6 !(A|B|C|D), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_A 66  //PK4, A10: X42, PIN5 (A), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_B 67  //PK5, A11: X42, PIN6 (B), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_A 68  //PK6, A14: X41, PIN5 (A), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_B 69  //PK7, A15: X41, PIN6 (B), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
//D68=A14 (IN): WZW Näherungssensor Position 0 ???

//for use with analogRead(Analog-PIN-NR)
#define APIN_SERVO_CONTROL_POTI 0    //PF0, A0, D54: CONTROL-POTI to manually set revolutions (Analog-IN)

//EEPROM-Adresses
#define LAST_X_STEP_ADDRESS 0
#define LAST_Z_STEP_ADDRESS 1
#define LAST_TOOL_ADDRESS 3

//TIMER
#define CLK_TIMER1 3750000 //in 1/min
#define CLK_TIMER3 3750000 //in 1/min

//global vars
extern boolean debug;

//Cosinus LookUp-Table for Quarter Circle in Q15
extern volatile const int lookup_cosinus[91];

//ERROR-Numbers
extern volatile byte ERROR_NO;

//Machine State
extern volatile byte STATE; //bit7_stepper|bit6_spindle_direction|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
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

