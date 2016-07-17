#ifndef Raspi_SPI_h
#define Raspi_SPI_h

//includes
#include <Arduino.h>
#include "CNC_Lathe.h"
#include "Motion_Control.h"
#include "CNC_Control.h"
#include "Spindle_Control.h"
#include "Tool_Changer_Control.h"
#include <SPI.h>

//defines
#define SPI_MSG_LENGTH 17

extern volatile boolean byte_received;
extern char tx_buf [100]; //SPI send-buffer

//Controlled by (Timer-) Interrupts:
void spi_buffer_handling();
char CRC8 (char *, char);
boolean process_incomming_msg();
void create_machine_state_msg();
void create_spi_error_msg();
//void send_error_number(char);
ISR (SPI_STC_vect);

#endif

