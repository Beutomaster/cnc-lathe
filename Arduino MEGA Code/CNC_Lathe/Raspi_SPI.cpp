/*
SPI-Transferprotokoll Raspberry Pi - Arduino
Bytes: PID length Parameter1_H Parameter1_L Parameter2_H ... CRC-8
lenght for every PID known! use case instead of sending it!!!

000 3 CRC-8 #Transfererror of last Block

To Arduino:
001 5 NN CRC-8 #Programm Start at Block
002 3 CRC-8 #Programm Stop
003 3 CRC-8 #Programm Pause
004 5 RPM_H&L CRC-8 #Spindle on with RPM
005 3 CRC-8 #Spindle off
006 5 FF CRC-8 #Stepper on with Feed
007 3 CRC-8 #Stepper off
008 4 T CRC-8 #Set Tool-Position (and INIT)
009 7 XX ZZ CRC-8 #Origin-Offset
010 4 metric CRC-8 #metric or inch
011 6 NN metric CRC-8 #New CNC-Programm wit NN Blocks in metric or inch
012 15 NN GG XX ZZ FF HH CRC-8 #CNC-Code-Block (6 Byte im 8kB Speicher pro Zeile CNC-Code)

From Arduino:
100 18 byte2=bit5_stepper|bit4_spindle|bit3_inch|bit2_manual|bit1_init|bit0_control_active RPM_H&L XX ZZ FF HH T NN ERROR_Numbers CRC-8 #Machine State
(101 4 ERROR_Number CRC-8 #Error) obsolete
*/

#include "CNC_Lathe.h"
#include "Raspi_SPI.h"
#include "Motion_Control.h"
#include "Spindle_Control.h"

char rx_buf [100]; //SPI receive-buffer
char tx_buf [100]; //SPI send-buffer
volatile byte pos=0; // buffer empty
volatile boolean byte_received=false; //first byte of transmission received)
volatile boolean process_it=false; //not end of string (newline received)

//Controlled by (Timer-) Interrupts:

// SPI-Transmission - wait for flag set in interrupt routine
void spi_buffer_handling() {
  if (process_it){
    rx_buf [pos] = 0;
    
    //for debugging
    Serial.println (rx_buf);
    set_revolutions(get_SERVO_CONTROL_POTI());
    set_xz_move(50, 50, 15, 0); //feed noch in rpm
    
    pos = 0;
    byte_received = false; //reset flag
    process_it = false; //reset flag
  }
} //end of receive_spi

char CRC8 (char * buf, char len) {
  char crc_8 = 0; //stub
  return crc_8;
}

void process_incomming_msg() {
}

char receive_cnc_code() {
	char cnc_code_received=0; //stub
	return cnc_code_received;
}

void receive_control_signal() {
	process_control_signal();
}

void create_machine_state_msg() {
  tx_buf [0] = 100; //PID
  tx_buf [1] = STATE; //bit5_stepper|bit4_spindle|bit3_inch|bit2_manual|bit1_init|bit0_control_active
  tx_buf [2] = STATE_RPM>>8;
  tx_buf [3] = STATE_RPM;
  tx_buf [4] = STATE_X>>8;
  tx_buf [5] = STATE_X;
  tx_buf [6] = STATE_Z>>8;
  tx_buf [7] = STATE_Z;
  tx_buf [8] = STATE_F>>8;
  tx_buf [9] = STATE_F;
  tx_buf [10] = STATE_H>>8;
  tx_buf [11] = STATE_H;
  tx_buf [12] = STATE_T; //0 = uninitialized
  tx_buf [13] = STATE_N>>8;
  tx_buf [14] = STATE_N;
  tx_buf [15] = ERROR_NO; //bit2_SPINDLE|bit1_CNC_CODE|bit0_SPI
  tx_buf [16] = CRC8(tx_buf, SPI_MSG_LENGTH);
  SPDR = tx_buf [0]; //first byte for sending at next interrupt
}

/* obsolete
void send_error_number(char error_number) {
  tx_buf [0] = 101; //PID
  tx_buf [1] = 3; //length
  tx_buf [2] = error_number;
  tx_buf [3] = CRC8(tx_buf, 3);
}  //end of send_error_number
*/

// SPI interrupt routine
ISR (SPI_STC_vect) {
  byte c = SPDR;  // grab byte from SPI Data Register
  byte_received = true;
  
  // add to rx_buffer if room and write next tx_buffer-value in SPDR-Register
  if (pos < sizeof rx_buf) {
    rx_buf [pos++] = c; //received byte
    SPDR = tx_buf [pos]; //byte for sending at next interrupt

    /*
    // newline means time to process buffer
    if (c == '\n') {
      process_it = true;
    }
    */ 
  }  // end of room available

  //process buffer at fixed SPI_MSG_LENGTH
  if (pos == (SPI_MSG_LENGTH) ) {
    process_it = true;
  }
  
}  // end of interrupt routine SPI_STC_vect

