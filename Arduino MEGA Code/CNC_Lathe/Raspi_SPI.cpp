#include "Raspi_SPI.h"

/*
SPI-Transferprotokoll Raspberry Pi - Arduino
Bytes: PID Parameter1_H Parameter1_L Parameter2_H ... CRC-8
lenght for every PID known! use case instead of sending it!!! (obsolete)
=> now using fixed Message-Lentgh of 17 bytes
unused bytes are filled with zeros

000 msg_no 15xZero CRC-8 #Update Machine State

maybe a Praeambel with 0xFF 0xFF (problematic because of 2s-complement) is needed, if bytes get lost, to synchronise again
RASPI sends a Message-Counter, the Arduino repeats it. so lost messages could be detected

maybe messages needed for:
#Transfererror of last Block or Message lost

To Arduino:
001 msg_no NN 13xZero CRC-8 #Programm Start at Block
002 msg_no 15xZero CRC-8 #Programm Stop
003 msg_no 15xZero CRC-8 #Programm Pause
004 msg_no RPM_H&L Direction 12xZero CRC-8 #Spindle on with RPM
005 msg_no 15xZero CRC-8 #Spindle off
006 msg_no 15xZero CRC-8 #Stepper on
007 msg_no 15xZero CRC-8 #Stepper off
008 msg_no FF negativ_direction 12xZero CRC-8 #X-Stepper move with feed
009 msg_no FF negativ_direction 12xZero CRC-8 #Z-Stepper move with feed
010 msg_no XX ZZ T 10xZero CRC-8 #Set Tool-Position (and INIT)
011 msg_no XX ZZ 11xZero CRC-8 #Origin-Offset
012 msg_no metric 14xZero CRC-8 #metric or inch (maybe not needed)
013 msg_no NN metric 12xZero CRC-8 #New CNC-Programm wit NN Blocks in metric or inch
014 msg_no NN GG XX ZZ FF HH 3xZero CRC-8 #CNC-Code-Block (6 Byte im 8kB Speicher pro Zeile CNC-Code)
015 msg_no 15xZero CRC-8 #shutdown
016 msg_no 15xZero CRC-8 #Reset Errors

//Transmission-Factor needed for Calculation of Revolutions !!!
//Change Spindle-Direction

From Arduino:
100 lastsuccessful_msg byte2=bit7_stepper|bit6_spindle_direction|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active RPM_H&L XX ZZ FF HH T NN ERROR_Numbers CRC-8 #Machine State
(101 ERROR_Number 14xZero CRC-8 #Error) obsolete
*/

char rx_buf [SPI_MSG_LENGTH]; //SPI receive-buffer
char rx_doublebuf [SPI_MSG_LENGTH]; //SPI receive-double-buffer
char tx_buf [SPI_MSG_LENGTH]; //SPI send-buffer
volatile byte pos=0; // buffer empty
volatile boolean byte_received=false; //first byte of transmission received)
volatile boolean process_it=false; //not end of string (newline received)
volatile unsigned char lastsuccessful_msg = 0;

//Controlled by (Timer-) Interrupts:

// SPI-Transmission - wait for flag set in interrupt routine
void spi_buffer_handling() {
  if (process_it){
    //rx_buf [pos] = 0; //set end of string

    //copy rx_buf to rx_doublebuf
    for (int i=0; i<SPI_MSG_LENGTH; i++) {
      rx_doublebuf[i] = rx_buf[i];
    }
    
    //Debug
    //String debug_string;
    if (debug && debug_spi) { //for debugging
      Serial.print("SPI-Buffer:");
      for (int i=0; i<pos; i++) {
        Serial.print(" ");
        //debug_string =  String(rx_doublebuf[i], DEC);
        //Serial.print(debug_string);
        Serial.print(rx_doublebuf[i], HEX);
      }
      Serial.println();
    }

    process_incomming_msg();
    create_machine_state_msg();

    pos = 0;
    byte_received = false; //reset flag
    process_it = false; //reset flag
  }
} //end of receive_spi

unsigned char CRC8 (char * buf, unsigned char used_message_bytes, boolean verify_with_extra_byte, unsigned char msg_crc_8) {
  //Set verify_with_extra_byte=false and msg_crc_8=0 (or whatever) to get the crc_8-value of the msg returned
  //If the last byte of the message is the correct crc-value of the bytes before, CRC8 returns 0.
  //Set verify_with_extra_byte=true and msg_crc_8=crc_8 of the message (byte 17) to verify the used message bytes against it. 
  //used_message_bytes is needed, because the crc_8 is only transmitted for used bytes.
  unsigned char bytecount, data, crc_8=0;
  
  for (bytecount=0; bytecount<used_message_bytes; bytecount++) {
    data = (unsigned char) buf[bytecount];
    crc_8 = _crc8_ccitt_update (crc_8,data);
  }
  if (verify_with_extra_byte) crc_8 = _crc8_ccitt_update (crc_8,msg_crc_8);
  
  return crc_8;
}

boolean check_msg(char msg_length, boolean force_action) {
  boolean success = true;
  if (CRC8(rx_doublebuf, msg_length, true, rx_doublebuf[SPI_BYTE_CRC8])) {
    success=false; //msg-failure
    set_error(ERROR_SPI_BIT);
  }
  else if (rx_doublebuf[SPI_BYTE_RASPI_MSG_NO] == lastsuccessful_msg+1) { //no message lost
    lastsuccessful_msg++;
  }
  else {
    if (force_action) success=false; //message lost
    set_error(ERROR_SPI_BIT);
  }
  return success;
}

boolean process_incomming_msg() {
  boolean success=true;
  int N=0;
  char msg_length=2;
  switch(rx_doublebuf[SPI_BYTE_PID]) {
    case 0:   //Update Machine State
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              break;
    case 1:   //Programm Start at Block
              msg_length=4;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (command_completed && (STATE>>STATE_PAUSE_BIT)&1) {
                programm_start((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_N_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_N_L]);
              }
              break;
    case 2:   //Programm Stop
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else programm_stop();
              break;
    case 3:   //Programm Pause
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else programm_pause();
              break;
    case 4:   //Spindle on with RPM and Direction
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                set_revolutions((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_RPM_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_RPM_L]);
                spindle_direction(rx_doublebuf[SPI_BYTE_RASPI_MSG_DIRECTION]);
                spindle_on();
              }
              break;
    case 5:   //Spindle off
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                spindle_off();
              }
              break;
    case 6:   //Stepper on
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                if (command_completed) { //Error Handling needed
                  stepper_on();
                }
              }
              break;
    case 7:   //Stepper off
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                if (command_completed) { //Error Handling needed
                  stepper_off();
                }
              }
              break;
    case 8:   //X-Stepper move with feed
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (command_completed && (STATE>>STATE_MANUAL_BIT)&1) {
                set_xz_stepper_manual((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_F_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_F_L], rx_doublebuf[SPI_BYTE_RASPI_MSG_DIRECTION], 0);
              }
              break;
    case 9:   //Z-Stepper move with feed
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (command_completed && (STATE>>STATE_PAUSE_BIT)&1) {
                set_xz_stepper_manual((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_F_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_F_L], rx_doublebuf[SPI_BYTE_RASPI_MSG_DIRECTION], 1);
              }
              break;
    case 10:   //Set Tool-Position (and INIT)
              msg_length=7;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (command_completed && (STATE>>STATE_MANUAL_BIT)&1) {
                get_Tool_X((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_X_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_X_L]);
                get_Tool_Z((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_Z_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_Z_L]);
                set_tool_position(rx_doublebuf[SPI_BYTE_RASPI_MSG_T]);
              }
              break;
    case 11:   //Origin-Offset
              msg_length=6;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (command_completed && (STATE>>STATE_PAUSE_BIT)&1) { //Error Handling needed
                set_xz_coordinates(((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_X_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_X_L]), ((((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_Z_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_Z_L]));
              }
              break;
    case 12:  //metric or inch (maybe not needed)
              msg_length=3;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (command_completed && (STATE>>STATE_PAUSE_BIT)&1) { //Error Handling needed
                if (rx_doublebuf[SPI_BYTE_RASPI_MSG_INCH]){
                  STATE |= _BV(STATE_INCH_BIT); //set STATE_bit4 = 1
                }
                else STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
              }
              break;
    case 13:  //New CNC-Programm wit N Blocks in metric or inch
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                //some Error-Handling needed, if message is ignored
                programm_stop();
                //int NMAX = ((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_N_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_N_L]) //maybe not needed
                for (N=0; N<CNC_CODE_NMAX; N++) {
                  cnc_code[N].GM = 0;
                }
                //metric or inch? maybe better replaced by G-Code
                if (rx_doublebuf[SPI_BYTE_RASPI_MSG_G_INCH]){
                  STATE |= _BV(STATE_INCH_BIT); //set STATE_bit4 = 1
                }
                else STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
              }
              else success=1;
              break;
    case 14:  //CNC-Code-Block
              msg_length=14;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                //some Error-Handling needed, if message is ignored
                N = (((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_N_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_N_L];
                cnc_code[N].GM = rx_doublebuf[SPI_BYTE_RASPI_MSG_GM];
                cnc_code[N].GM_NO = rx_doublebuf[SPI_BYTE_RASPI_MSG_GM_NO];
                cnc_code[N].XI = (((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_XI_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_XI_L];
                cnc_code[N].ZK = (((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_ZK_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_ZK_L];
                cnc_code[N].FTLK = (((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_FTLK_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_FTLK_L];
                cnc_code[N].HS = (((int)rx_doublebuf[SPI_BYTE_RASPI_MSG_HS_H])<<8) | rx_doublebuf[SPI_BYTE_RASPI_MSG_HS_L];
              }
              else success=1;
              break;
    case 15:  //Shutdown
              if (!check_msg(msg_length, true)) success=false; //msg-failure
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
              if (!check_msg(msg_length, true)) success=false; //msg-failure
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
  tx_buf [SPI_BYTE_PID] = 100; //PID
  tx_buf [SPI_BYTE_LASTSUCCESS_MSG_NO] = lastsuccessful_msg;
  tx_buf [SPI_BYTE_STATE] = STATE; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
  tx_buf [SPI_BYTE_RPM_H] = STATE_RPM>>8;
  tx_buf [SPI_BYTE_RPM_L] = STATE_RPM;
  tx_buf [SPI_BYTE_X_H] = STATE_X>>8;
  tx_buf [SPI_BYTE_X_L] = STATE_X;
  tx_buf [SPI_BYTE_Z_H] = STATE_Z>>8;
  tx_buf [SPI_BYTE_Z_L] = STATE_Z;
  tx_buf [SPI_BYTE_F_H] = STATE_F>>8;
  tx_buf [SPI_BYTE_F_L] = STATE_F;
  tx_buf [SPI_BYTE_H_H] = STATE_H>>8;
  tx_buf [SPI_BYTE_H_L] = STATE_H;
  tx_buf [SPI_BYTE_T] = STATE_T; //0 = uninitialized
  tx_buf [SPI_BYTE_N_H] = STATE_N>>8;
  tx_buf [SPI_BYTE_N_L] = STATE_N;
  tx_buf [SPI_BYTE_ERROR_NO] = ERROR_NO; //bit2_SPINDLE|bit1_CNC_CODE|bit0_SPI
  tx_buf [SPI_BYTE_CRC8] = CRC8(tx_buf, SPI_MSG_LENGTH-1, false, 0);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //needed? 8bit?
    SPDR = tx_buf [0]; //first byte for sending at next interrupt
  }
}

void create_other_msg() { //maybee not needed
  tx_buf [SPI_BYTE_PID] = 101; //PID
  tx_buf [SPI_BYTE_LASTSUCCESS_MSG_NO] = lastsuccessful_msg;
  for (int i=2; i<(SPI_MSG_LENGTH-2); i++) {
    tx_buf [i] = 0;
  }
  tx_buf [SPI_MSG_LENGTH-1] = CRC8(tx_buf, 2, false, 0);
  SPDR = tx_buf [0]; //first byte for sending at next interrupt
}

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

