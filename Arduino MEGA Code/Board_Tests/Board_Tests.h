#ifndef Board_Tests_h
#define Board_Tests_h

//includes
#include <Arduino.h>
#include "PIN_Test_Stepper.h"


//PINs
#define PIN_CONTROL_INACTIVE 2       //PE4: Switch between EMCO and alternative Control (Usable for extINTR !!!)
#define PIN_REVOLUTIONS_SYNC 3     //PE5: Revolution-Sensor SYNC (Usable for extINTR !!!)
#define PIN_TOOL_CHANGER_HOLD 4     //PG5: Tool-Changer hold (-3,3V)
#define PIN_TOOL_CHANGER_CHANGE 5     //PE3: Tool-Changer change (+12,9V)
#define PIN_TOOL_CHANGER_FIXING 6     //PH3: Tool-Changer fixing (-4,35V)
#define PIN_SPINDLE_ON 7          //PH4: Spindle on
#define PIN_SPINDELPWM_NIKO 8           //PH5: Timer5 Fast PWM (OC4C) for Niko's spindle driver (set Revolutions)
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
#define PIN_SPINDELBOARD_NIKO 59    //PF5: Switch between Niko's Board and Hannes Servo
#define PIN_REVOLUTIONS_COUNT 62    //PK0, A08: Revolution-Sensor COUNT
#define PIN_OLD_CONTROL_STEPPER_X_OFF 65 //PK3, A09: X42, PIN3,4,5,6 !(A|B|C|D), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_A 66  //PK4, A10: X42, PIN5 (A), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_X_B 67  //PK5, A11: X42, PIN6 (B), Stepper X (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_A 68  //PK6, A14: X41, PIN5 (A), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
#define PIN_OLD_CONTROL_STEPPER_Z_B 69  //PK7, A15: X41, PIN6 (B), Stepper Z (Watching old Control) (NOT Usable for extINTR !!!)
//D68=A14 (IN): WZW Näherungssensor Position 0 ???

//for use with analogRead(Analog-PIN-NR)
#define APIN_SERVO_CONTROL_POTI 0    //PF0, A0, D54: CONTROL-POTI to manually set revolutions (Analog-IN)

#endif

