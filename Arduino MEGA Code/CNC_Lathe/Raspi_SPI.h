#ifndef Raspi_SPI_h
#define Raspi_SPI_h

#include <SPI.h>

#define SPI_MSG_LENGTH 17

extern volatile boolean byte_received;
extern char tx_buf [100]; //SPI send-buffer

//Controlled by (Timer-) Interrupts:
void spi_buffer_handling();
char CRC8 (char *, char);
char process_incomming_msg();
char receive_cnc_code();
void receive_control_signal();
void create_machine_state_msg();
void send_spi_error();
//void send_error_number(char);
ISR (SPI_STC_vect);

#endif

