#include "Board_Tests.h"

int i=0;

void setup() {
  // put your setup code here, to run once:
  //PINs
  pinMode(PIN_CONTROL_INACTIVE, INPUT_PULLUP); //LOW-Active (GND = Control activ)
  pinMode(PIN_REVOLUTIONS_SYNC, INPUT);
  pinMode(PIN_REVOLUTIONS_COUNT, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_OFF, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_X_B, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_A, INPUT);
  pinMode(PIN_OLD_CONTROL_STEPPER_Z_B, INPUT);
  //pinMode(PIN_SERVO_ENGINE, OUTPUT); //needed for Fast PWM
  //pinMode(PIN_SPINDELBOARD_NIKO, OUTPUT);
  //pinMode(PIN_STEPPER_X_A, OUTPUT);
  //pinMode(PIN_STEPPER_X_B, OUTPUT);
  //pinMode(PIN_STEPPER_X_C, OUTPUT);
  //pinMode(PIN_STEPPER_X_D, OUTPUT);
  //pinMode(PIN_STEPPER_Z_A, OUTPUT);
  //pinMode(PIN_STEPPER_Z_B, OUTPUT);
  //pinMode(PIN_STEPPER_Z_C, OUTPUT);
  //pinMode(PIN_STEPPER_Z_D, OUTPUT);
  //pinMode(PIN_TOOL_CHANGER_HOLD, OUTPUT);
  //pinMode(PIN_TOOL_CHANGER_CHANGE, OUTPUT);
  //pinMode(PIN_TOOL_CHANGER_FIXING, OUTPUT);
  //pinMode(PIN_SPINDLE_ON, OUTPUT);
  //pinMode(PIN_SPINDLE_DIRECTION, OUTPUT);
  pinMode(PIN_USART1_RX, INPUT);
  //pinMode(PIN_USART1_TX, OUTPUT);
  //pinMode(PIN_SPI_MISO, OUTPUT); 		//Arduino is SPI-Slave
  //pinMode(PIN_SPI_MOSI, INPUT); 	//Arduino is SPI-Slave
  //pinMode(PIN_SPI_SCK, INPUT); 	//Arduino is SPI-Slave
  //pinMode(PIN_SPI_SS, INPUT); 	//Arduino is SPI-Slave
  //potiservo.attach(PIN_SERVO_ENGINE);   //Attach Servo-Pin
  
  //Serial Communication
  Serial.begin(115200); //for Debugging with Serial Monitor
}

void loop() {
  // put your main code here, to run repeatedly:

  //Stepper-Pins
  stepper_off();
  delay(20);
  for (i=0; i<4; i++) {
    set_xstep(i);
    set_zstep(i);
    delay(20);
  }
}
