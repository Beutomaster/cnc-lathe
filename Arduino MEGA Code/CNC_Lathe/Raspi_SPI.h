#ifndef Raspi_SPI_h
#define Raspi_SPI_h

#include <SPI.h>

#define SPI_MSG_LENGTH 17

extern volatile boolean byte_received;

//Controlled by (Timer-) Interrupts:
void spi_buffer_handling();
char CRC8 (char *, char);
void process_incomming_msg();
char receive_cnc_code();
void receive_control_signal();
void create_machine_state_msg();
//void send_error_number(char);
ISR (SPI_STC_vect);

#endif

