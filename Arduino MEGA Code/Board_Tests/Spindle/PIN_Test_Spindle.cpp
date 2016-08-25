#include "PIN_Test_Stepper.h"

void stepper_off() {
  //A,B,C,D LOW (We have to change the logic, because Signals at the moment are always C=!A, D=!B !!!)
  digitalWrite(PIN_STEPPER_X_A, LOW);
  digitalWrite(PIN_STEPPER_X_B, LOW);
  digitalWrite(PIN_STEPPER_X_C, LOW);
  digitalWrite(PIN_STEPPER_X_D, LOW);
  digitalWrite(PIN_STEPPER_Z_A, LOW);
  digitalWrite(PIN_STEPPER_Z_B, LOW);
  digitalWrite(PIN_STEPPER_Z_C, LOW);
  digitalWrite(PIN_STEPPER_Z_D, LOW);
}

void set_xstep(byte nextstep) {
  //+X/+Z: B 90° before A,
  //-X/-Z: A 90° before B,
  //C=!A, D=!B
  switch (nextstep) {
      case 0:  // 0011
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, LOW);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, HIGH);
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_X_A, LOW);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, HIGH);
        digitalWrite(PIN_STEPPER_X_D, LOW);
      break;
      case 2:  //1100
        digitalWrite(PIN_STEPPER_X_A, HIGH);
        digitalWrite(PIN_STEPPER_X_B, HIGH);
        digitalWrite(PIN_STEPPER_X_C, LOW);
        digitalWrite(PIN_STEPPER_X_D, LOW);
      break;
      case 3:  //1001
        digitalWrite(PIN_STEPPER_X_A, HIGH);
        digitalWrite(PIN_STEPPER_X_B, LOW);
        digitalWrite(PIN_STEPPER_X_C, LOW);
        digitalWrite(PIN_STEPPER_X_D, HIGH);
    }
}

void set_zstep(byte nextstep) {
  //+X/+Z: B 90° before A,
  //-X/-Z: A 90° before B,
  //C=!A, D=!B
  switch (nextstep) {
      case 0:  // 0011
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, LOW);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, HIGH);
      break;
      case 1:  // 0110
        digitalWrite(PIN_STEPPER_Z_A, LOW);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, HIGH);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
      break;
      case 2:  //1100
        digitalWrite(PIN_STEPPER_Z_A, HIGH);
        digitalWrite(PIN_STEPPER_Z_B, HIGH);
        digitalWrite(PIN_STEPPER_Z_C, LOW);
        digitalWrite(PIN_STEPPER_Z_D, LOW);
      break;
      case 3:  //1001
        digitalWrite(PIN_STEPPER_Z_A, HIGH);
        digitalWrite(PIN_STEPPER_Z_B, LOW);
        digitalWrite(PIN_STEPPER_Z_C, LOW);
        digitalWrite(PIN_STEPPER_Z_D, HIGH);
    }
}
