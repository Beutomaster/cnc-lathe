#include "Raspi_SPI.h"

/*
SPI-Transferprotokoll Raspberry Pi - Arduino
Bytes: PID length Parameter1_H Parameter1_L Parameter2_H ... CRC-8
lenght for every PID known! use case instead of sending it!!!

000 3 CRC-8 #Transfererror of last Block

To Arduino:
001 5 NN CRC-8 #Programm Start at Block
002 3 CRC-8 #Programm Stop
003 3 CRC-8 #Programm Pause
004 5 RPM_H&L Direction CRC-8 #Spindle on with RPM
005 3 CRC-8 #Spindle off
006 5 CRC-8 #Stepper on
007 3 CRC-8 #Stepper off
008 5 FF negativ_direction CRC-8 #X-Stepper move with feed
009 5 FF negativ_direction CRC-8 #Z-Stepper move with feed
010 4 T CRC-8 #Set Tool-Position (and INIT)
011 7 XX ZZ CRC-8 #Origin-Offset
012 4 metric CRC-8 #metric or inch (maybe not needed)
013 6 NN metric CRC-8 #New CNC-Programm wit NN Blocks in metric or inch
014 15 NN GG XX ZZ FF HH CRC-8 #CNC-Code-Block (6 Byte im 8kB Speicher pro Zeile CNC-Code)
015 CRC-8 #shutdown

//Transmission-Factor needed for Calculation of Revolutions !!!
//Change Spindle-Direction

From Arduino:
100 18 byte2=bit5_stepper|bit4_spindle|bit3_inch|bit2_manual|bit1_init|bit0_control_active RPM_H&L XX ZZ FF HH T NN ERROR_Numbers CRC-8 #Machine State
(101 4 ERROR_Number CRC-8 #Error) obsolete
*/

char rx_buf [SPI_MSG_LENGTH]; //SPI receive-buffer
char tx_buf [SPI_MSG_LENGTH]; //SPI send-buffer
volatile byte pos=0; // buffer empty
volatile boolean byte_received=false; //first byte of transmission received)
volatile boolean process_it=false; //not end of string (newline received)

//Controlled by (Timer-) Interrupts:

// SPI-Transmission - wait for flag set in interrupt routine
void spi_buffer_handling() {
  if (process_it){
    //rx_buf [pos] = 0; //set end of string

    /*
    //for debugging
    Serial.println (rx_buf);
    set_revolutions(get_SERVO_CONTROL_POTI());
    set_xz_move(50, 50, 15, 0); //feed noch in rpm
    */

    if (process_incomming_msg()) create_spi_error_msg();
    else create_machine_state_msg();
    
    pos = 0;
    byte_received = false; //reset flag
    process_it = false; //reset flag
  }
} //end of receive_spi

char CRC8 (char * buf, char len) {
  char crc_8 = 0; //stub
  return crc_8;
}

boolean process_incomming_msg() {
  boolean success=0; //0=success, 1=failure
  int N=0;
  switch(rx_buf[0]) {
    case 0:   //Transfererror of last Block
              break;
    case 1:   //Programm Start at Block
              if ((STATE>>STATE_PAUSE_BIT)&1) {
                programm_start((((int)rx_buf[1])<<8) + rx_buf[2]);
              }
              break;
    case 2:   //Programm Stop
              programm_stop();
              break;
    case 3:   //Programm Pause
              programm_pause();
              break;
    case 4:   //Spindle on with RPM and Direction
              if ((STATE>>STATE_MANUAL_BIT)&1) {
                set_revolutions((((int)rx_buf[1])<<8) + rx_buf[2]);
                spindle_direction(rx_buf[3]);
                spindle_on();
              }
              break;
    case 5:   //Spindle off
              if ((STATE>>STATE_MANUAL_BIT)&1) {
                spindle_off();
              }
              break;
    case 6:   //Stepper on
              if ((STATE>>STATE_MANUAL_BIT)&1) {
                stepper_on();
              }
              break;
    case 7:   //Stepper off
              if ((STATE>>STATE_MANUAL_BIT)&1) {
                stepper_off();
              }
              break;
    case 8:   //X-Stepper move with feed
              if ((STATE>>STATE_MANUAL_BIT)&1) {
                set_xstepper((((int)rx_buf[1])<<8) + rx_buf[2], rx_buf[3]);
              }
              break;
    case 9:   //Z-Stepper move with feed
              if ((STATE>>STATE_PAUSE_BIT)&1) {
                set_zstepper((((int)rx_buf[1])<<8) + rx_buf[2], rx_buf[3]);
              }
    case 10:   //Set Tool-Position (and INIT)
              if ((STATE>>STATE_MANUAL_BIT)&1) {
                set_tool_position(rx_buf[1]);
              }
              break;
    case 11:   //Origin-Offset
              if ((STATE>>STATE_PAUSE_BIT)&1) {
                set_xz_coordinates(((((int)rx_buf[1])<<8) + rx_buf[2]), ((((int)rx_buf[3])<<8) + rx_buf[4]));
              }
              break;
    case 12:  //metric or inch (maybe not needed)
              if ((STATE>>STATE_MANUAL_BIT)&1) {
                if (rx_buf[1]){
                  STATE |= _BV(STATE_INCH_BIT); //set STATE_bit4 = 1
                }
                else STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
              }
              break;
    case 13:  //New CNC-Programm wit N Blocks in metric or inch
              if ((STATE>>STATE_PAUSE_BIT)&1) {
                //stub
                //some Error-Handling needed
              }
              break;
    case 14:  //CNC-Code-Block
              if ((STATE>>STATE_PAUSE_BIT)&1) {
                N = (((int)rx_buf[1])<<8) + rx_buf[2];
                cnc_code[N].GM = rx_buf[3];
                cnc_code[N].GM_NO = rx_buf[4];
                cnc_code[N].XI = (((int)rx_buf[5])<<8) + rx_buf[6];
                cnc_code[N].ZK = (((int)rx_buf[7])<<8) + rx_buf[8];
                cnc_code[N].FTLK = (((int)rx_buf[9])<<8) + rx_buf[10];
                cnc_code[N].HS = (((int)rx_buf[11])<<8) + rx_buf[12];
              }
              break;
    case 15:  //Shutdown
              programm_stop();
              stepper_off();
              spindle_off();
              save_current_x_step();
              save_current_z_step();
              save_current_tool_position();
              break;
    default:  //SPI-Error "PID unkown"
              success=1;
  }
  return success;
}

void create_machine_state_msg() {
  tx_buf [0] = 100; //PID
  tx_buf [1] = STATE; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
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

void create_spi_error_msg() {
  tx_buf [0] = 0; //PID
  tx_buf [2] = CRC8(tx_buf, 2);
}  //end of send_spi_error

/* obsolete
void send_error_number(byte error_number) {
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
    /*
    // newline means time to process buffer
    if (c == '\n') {
      process_it = true;
    }
    */ 
  }  // end of room available

  if (pos < sizeof tx_buf) {
    SPDR = tx_buf [pos]; //byte for sending at next interrupt
  }
  
  //process buffer at fixed SPI_MSG_LENGTH
  if (pos == (SPI_MSG_LENGTH) ) {
    process_it = true;
  }
  
}  // end of interrupt routine SPI_STC_vect

