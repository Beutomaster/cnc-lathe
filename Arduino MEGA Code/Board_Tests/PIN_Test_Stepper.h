#ifndef PIN_Test_Stepper_h
#define PIN_Test_Stepper_h

#include <util/atomic.h>
#include <Arduino.h>
#include "Board_Tests.h"

void stepper_off();
void set_xstep(byte);
void set_zstep(byte);

#endif

