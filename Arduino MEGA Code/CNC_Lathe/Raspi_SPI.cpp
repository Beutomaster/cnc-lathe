#include "Raspi_SPI.h"

/*
SPI-Transferprotokoll Raspberry Pi - Arduino
Bytes: PID Parameter1_H Parameter1_L Parameter2_H ... CRC-8
lenght for every PID known! use case instead of sending it!!! (obsolete)
=> now using fixed Message-Lentgh of 17 bytes
unused bytes are filled with zeros

000 15xZero CRC-8 #Update Machine State

maybe messages needed for:
#Transfererror of last Block


To Arduino:
001 NN 13xZero CRC-8 #Programm Start at Block
002 15xZero CRC-8 #Programm Stop
003 15xZero CRC-8 #Programm Pause
004 RPM_H&L Direction 12xZero CRC-8 #Spindle on with RPM
005 15xZero CRC-8 #Spindle off
006 15xZero CRC-8 #Stepper on
007 15xZero CRC-8 #Stepper off
008 FF negativ_direction 12xZero CRC-8 #X-Stepper move with feed
009 FF negativ_direction 12xZero CRC-8 #Z-Stepper move with feed
010 XX ZZ T 10xZero CRC-8 #Set Tool-Position (and INIT)
011 XX ZZ 11xZero CRC-8 #Origin-Offset
012 metric 14xZero CRC-8 #metric or inch (maybe not needed)
013 NN metric 12xZero CRC-8 #New CNC-Programm wit NN Blocks in metric or inch
014 NN GG XX ZZ FF HH 3xZero CRC-8 #CNC-Code-Block (6 Byte im 8kB Speicher pro Zeile CNC-Code)
015 15xZero CRC-8 #shutdown
016 15xZero CRC-8 #Reset Errors

//Transmission-Factor needed for Calculation of Revolutions !!!
//Change Spindle-Direction

From Arduino:
100 byte2=bit7_stepper|bit6_spindle_direction|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active RPM_H&L XX ZZ FF HH T NN ERROR_Numbers CRC-8 #Machine State
(101 ERROR_Number 14xZero CRC-8 #Error) obsolete
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
    
    //Debug
    //String debug_string;
    if (debug && debug_spi) { //for debugging
      Serial.print("SPI-Buffer:");
      for (int i=0; i<pos; i++) {
        Serial.print(" ");
        //debug_string =  String(rx_buf[i], DEC);
        //Serial.print(debug_string);
        Serial.print(rx_buf[i], HEX);
      }
      Serial.println();
    }

    if (process_incomming_msg()) set_error(ERROR_SPI_BIT); //create_spi_error_msg();
    create_machine_state_msg();

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
  char msg_length=1;
  switch(rx_buf[0]) {
    case 0:   //Update Machine State
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              break;
    case 1:   //Programm Start at Block
              msg_length=3;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                programm_start((((int)rx_buf[1])<<8) | rx_buf[2]);
              }
              break;
    case 2:   //Programm Stop
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else programm_stop();
              break;
    case 3:   //Programm Pause
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else programm_pause();
              break;
    case 4:   //Spindle on with RPM and Direction
              msg_length=4;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                set_revolutions((((int)rx_buf[1])<<8) | rx_buf[2]);
                spindle_direction(rx_buf[3]);
                spindle_on();
              }
              break;
    case 5:   //Spindle off
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                spindle_off();
              }
              break;
    case 6:   //Stepper on
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                stepper_on();
              }
              break;
    case 7:   //Stepper off
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                stepper_off();
              }
              break;
    case 8:   //X-Stepper move with feed
              msg_length=4;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                set_xz_stepper_manual((((int)rx_buf[1])<<8) | rx_buf[2], rx_buf[3], 0);
              }
              break;
    case 9:   //Z-Stepper move with feed
              msg_length=4;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                set_xz_stepper_manual((((int)rx_buf[1])<<8) | rx_buf[2], rx_buf[3], 1);
              }
              break;
    case 10:   //Set Tool-Position (and INIT)
              msg_length=6;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                get_Tool_X((((int)rx_buf[1])<<8) | rx_buf[2]);
                get_Tool_Z((((int)rx_buf[3])<<8) | rx_buf[4]);
                set_tool_position(rx_buf[5]);
              }
              break;
    case 11:   //Origin-Offset
              msg_length=5;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                set_xz_coordinates(((((int)rx_buf[1])<<8) | rx_buf[2]), ((((int)rx_buf[3])<<8) | rx_buf[4]));
              }
              break;
    case 12:  //metric or inch (maybe not needed)
              msg_length=2;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                if (rx_buf[1]){
                  STATE |= _BV(STATE_INCH_BIT); //set STATE_bit4 = 1
                }
                else STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
              }
              break;
    case 13:  //New CNC-Programm wit N Blocks in metric or inch
              msg_length=4;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                //stub
                //some Error-Handling needed
              }
              else success=1;
              break;
    case 14:  //CNC-Code-Block
              msg_length=13;
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                N = (((int)rx_buf[1])<<8) | rx_buf[2];
                cnc_code[N].GM = rx_buf[3];
                cnc_code[N].GM_NO = rx_buf[4];
                cnc_code[N].XI = (((int)rx_buf[5])<<8) | rx_buf[6];
                cnc_code[N].ZK = (((int)rx_buf[7])<<8) | rx_buf[8];
                cnc_code[N].FTLK = (((int)rx_buf[9])<<8) | rx_buf[10];
                cnc_code[N].HS = (((int)rx_buf[11])<<8) | rx_buf[12];
              }
              else success=1;
              break;
    case 15:  //Shutdown
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else {
                programm_stop();
                stepper_off();
                spindle_off();
                save_current_x_step();
                save_current_z_step();
                save_current_tool_position();
              }
              break;
    case 16:  //Reset Errors
              if (CRC8(rx_buf, msg_length) != rx_buf[SPI_MSG_LENGTH-1]) success=1;
              else {
                ERROR_NO = 0;
              }
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
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //needed? 8bit?
    SPDR = tx_buf [0]; //first byte for sending at next interrupt
  }
}

void create_spi_error_msg() { //maybee not needed anymore, using error_bit
  //tx_buf [0] = 0; //PID
  for (int i=0; i<(SPI_MSG_LENGTH-2); i++) {
    tx_buf [i] = 0;
  }
  tx_buf [SPI_MSG_LENGTH-1] = CRC8(tx_buf, 2);
  SPDR = tx_buf [0]; //first byte for sending at next interrupt
}  //end of send_spi_error

/* obsolete
void send_error_number(byte error_number) {
  tx_buf [0] = 101; //PID
  tx_buf [1] = 3; //length
  tx_buf [2] = error_number;
  tx_buf [3] = CRC8(tx_buf, 3);
  SPDR = tx_buf [0]; //first byte for sending at next interrupt
}  //end of send_error_number
*/

// SPI interrupt routine
ISR (SPI_STC_vect) {
  byte c = SPDR;  // grab byte from SPI Data Register
  byte_received = true;
  
  // add to rx_buffer if room and write next tx_buffer-value in SPDR-Register
  if (pos < sizeof(rx_buf)) {
    rx_buf [pos] = c; //received byte
    /*
    // newline means time to process buffer
    if (c == '\n') {
      process_it = true;
    }
    */
  }  // end of room available

  /*
  if (debug && debug_spi) { //for debugging
      Serial.print("pos: ");
      Serial.println(pos);
  }
  */

  pos++;
  
  if (pos < sizeof(tx_buf)) {
    SPDR = tx_buf [pos]; //byte for sending at next interrupt
  }
  
  //process buffer at fixed SPI_MSG_LENGTH
  if (pos == SPI_MSG_LENGTH) {
    process_it = true;
  }
  
}  // end of interrupt routine SPI_STC_vect

