#ifndef Board_Tests_h
#define Board_Tests_h

//includes
#include <util/atomic.h>
#include <Arduino.h>
#include "PIN_Test_Stepper.h"
#include "PIN_Test_Spindle.h"
#include "PIN_Test_Tool_Changer.h"

//defines
//Code-Switches
//#define SERVO_LIB
//#define STEPPER_LIB
//#define SPINDLEDRIVER_EXTRA_BOARD
//#define BOARDVERSION_1_25 //else Bord V3.2
//#define SPINDLEDRIVER_NEW

//PINs
#define PIN_CONTROL_ACTIVE 2              //PE4: Switch between EMCO and alternative Control (Usable for extINTR !!!)
#define PIN_REVOLUTIONS_SYNC 3            //PE5: Revolution-Sensor SYNC (Usable for extINTR !!!) We should change the PIN with PIN_REVOLUTIONS_COUNT in V2!!!
#ifdef BOARDVERSION_1_25
  #define PIN_TOOL_CHANGER_HOLD 4           //PG5: Tool-Changer hold (-3,3V)
#endif
#define PIN_TOOL_CHANGER_CHANGE 5         //PE3: Tool-Changer change (+12,9V)
#ifndef BOARDVERSION_1_25
  #define PIN_TOOL_CHANGER_FIXING 6       //PH3: Tool-Changer fixing (-4,35V)
#endif
#define PIN_SPINDLE_ON 7                  //PH4: Spindle on
#define PIN_SPINDLEPWM_NIKO 8             //PH5: Timer4 Fast PWM (OC4C) for Niko's spindle driver (set Revolutions)
#define PIN_SPINDLE_DIRECTION 9           //PH6: HIGH=Inverse Direction
#define PIN_STEPPER_X_A 10                //PB4: X35, PIN5 (A), Stepper X
#define PIN_STEPPER_X_B 11                //PB5: X35, PIN6 (B), Stepper X
#define PIN_STEPPER_X_C 12                //PB6: X35, PIN3 (C), Stepper X
#define PIN_STEPPER_X_D 13                //PB7: X35, PIN4 (D), Stepper X
#define PIN_STEPPER_Z_A 14                //PJ1: X34, PIN5 (A), Stepper Z
#define PIN_STEPPER_Z_B 15                //PJ0: X34, PIN6 (B), Stepper Z
#ifdef BOARDVERSION_1_25
  #define PIN_STEPPER_Z_C 17 //Hotfix, planed was 16   //PH0: X34, PIN3 (C), Stepper Z
  #define PIN_STEPPER_Z_D 16 //Hotfix, planed was 17   //PH1: X34, PIN4 (D), Stepper Z
#else
  #define PIN_STEPPER_Z_C 16              //PH1: X34, PIN3 (C), Stepper Z
  #define PIN_STEPPER_Z_D 17              //PH0: X34, PIN4 (D), Stepper Z
#endif
#define PIN_USART1_TX 18                  //D18 (OUT) : Spindle-Board Niko: set RPM per USART
#define PIN_USART1_RX 19                  //D19 (IN): Spindle-Board Niko: set RPM per USART
#define PIN_SPINDLE_CHARGERESISTOR_OFF 38 //Bridge Charge Resisitor for New Spindle Capacitor
#define PIN_SERVO_ENGINE 46               //PL3: PWM for Servo (Poti to set Revolutions) Timer5 PWM on Pins 44,45,46 is occupied by Servo.h (but it is not necessary to choose these pins)
//#define PIN_SPI_MISO 50                 //D50 (OUT): SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
//#define PIN_SPI_MOSI 51                 //D51 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
//#define PIN_SPI_SCK 52                  //D52 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
//#define PIN_SPI_SS 53                   //D53 (IN) : SPI RaspBerry (Master) <-> Arduino (Slave) (now defined in AppData\Local\Arduino15\packages\arduino\hardware\avr\1.6.12\variants\mega/pins_arduino.h)
#define PIN_SPINDLE_ON_DETECT 55          //A01, PF1: Detect if Spindle-Switch is on
#define PIN_SPINDLE_NEW 59                //A05, PF5: Switch between Niko's Board and Hannes Servo
#define PIN_DEBUG_INPUT_STEPPER 60    //Debug Input 1
#define PIN_DEBUG_INPUT_WZW 61    //Debug Input 2
#define PIN_DEBUG_INPUT_SPINDLE 58 //Debug Input Spindle
#define PIN_REVOLUTIONS_COUNT 62          //A08, PK0: Revolution-Sensor COUNT
#define PIN_OLD_CONTROL_STEPPER_X_OFF 65  //A11, PK3: X42, PIN3,4,5,6 !(A|B|C|D), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_A 66    //A12, PK4: X42, PIN5 (A), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_B 67    //A13, PK5: X42, PIN6 (B), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_A 68    //A14, PK6: X41, PIN5 (A), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_B 69    //A15, PK7: X41, PIN6 (B), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
//D68=A14 (IN): WZW Näherungssensor Position 0 ???

//for use with analogRead(Analog-PIN-NR)
#define APIN_SERVO_CONTROL_POTI 0    //PF0, A0, D54: CONTROL-POTI to manually set revolutions (Analog-IN)

extern volatile byte STATE_T;
extern volatile boolean command_completed;
extern volatile boolean debug, debug_spi, debug_stepper, debug_active, debug_rpm, debug_tool;

#endif

