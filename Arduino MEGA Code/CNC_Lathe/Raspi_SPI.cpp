#include "Raspi_SPI.h"

/*
SPI-Transferprotokoll Raspberry Pi - Arduino
Bytes: PID Parameter1_H Parameter1_L Parameter2_H ... CRC-8
lenght for every PID known! use case instead of sending it!!! (obsolete)
=> now using fixed Message-Lentgh of 18 bytes + Praeambel
unused bytes are filled with zeros

a Praeambel with 0x7F 0xFF 0x7F 0xFF (problematic because of 2s-complement for signed values, unsigned values and possible combinations of single bytes) is needed, if bytes get lost, to synchronise again
RASPI sends a Message-Counter, the Arduino repeats it. so lost messages could be detected

maybe messages needed for:
#Transfererror of last Block or Message lost

To Arduino:
Praeambel 001 msg_no 15xZero CRC-8 #Update Machine State
Praeambel 002 msg_no NN 13xZero CRC-8 #Programm Start at Block
Praeambel 003 msg_no 15xZero CRC-8 #Programm Stop
Praeambel 004 msg_no 15xZero CRC-8 #Programm Pause
Praeambel 005 msg_no RPM_H&L Direction 12xZero CRC-8 #Spindle on with RPM
Praeambel 006 msg_no 15xZero CRC-8 #Spindle off
Praeambel 007 msg_no 15xZero CRC-8 #Stepper on
Praeambel 008 msg_no 15xZero CRC-8 #Stepper off
Praeambel 009 msg_no FF negativ_direction 12xZero CRC-8 #X-Stepper move with feed
Praeambel 010 msg_no FF negativ_direction 12xZero CRC-8 #Z-Stepper move with feed
Praeambel 011 msg_no XX ZZ T 10xZero CRC-8 #Set Tool-Position (and INIT)
Praeambel 012 msg_no XX 13xZero CRC-8 #Origin-XOffset
Praeambel 013 msg_no ZZ 13xZero CRC-8 #Origin-ZOffset
Praeambel 014 msg_no metric 14xZero CRC-8 #metric or inch (maybe not needed)
Praeambel 015 msg_no NN metric 12xZero CRC-8 #New CNC-Programm wit NN Blocks in metric or inch
Praeambel 016 msg_no NN GG XX ZZ FF HH 3xZero CRC-8 #CNC-Code-Block (6 Byte im 8kB Speicher pro Zeile CNC-Code)
Praeambel 017 msg_no 15xZero CRC-8 #shutdown
Praeambel 018 msg_no 15xZero CRC-8 #Load last coordinates and tool position and init
Praeambel 019 msg_no 15xZero CRC-8 #Reset Errors

//Transmission-Factor needed for Calculation of Revolutions !!!
//Change Spindle-Direction

From Arduino:
Praeambel 100 lastsuccessful_msg byte2=bit7_stepper|bit6_spindle_direction|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active RPM_H&L XX ZZ FF HH T NN ERROR_Numbers CRC-8 #Machine State
(Praeambel 101 ERROR_Number 14xZero CRC-8 #Error) obsolete
*/

volatile unsigned char rx_buf [SPI_MSG_LENGTH]; //SPI receive-buffer
volatile unsigned char rx_doublebuf [SPI_RX_RINGBUFFERSIZE][SPI_MSG_LENGTH-SPI_BYTE_LENGTH_PRAEAMBEL]; //SPI receive-double-buffer
volatile unsigned char tx_buf [SPI_MSG_LENGTH]; //SPI send-buffer
volatile unsigned char tx_doublebuf [SPI_MSG_LENGTH-SPI_BYTE_LENGTH_PRAEAMBEL]; //SPI send-double-buffer
volatile byte pos=0; // buffer empty
volatile boolean byte_received=false; //first byte of transmission received)
volatile boolean process_it=false; //not end of string (newline received)
volatile unsigned char lastsuccessful_msg = 0;
volatile char messages_to_process=0, rx_ringbuffer_write_pos=0, rx_ringbuffer_read_pos=0;

//Update messages_to_process
void update_messages_to_process() {
  if ((rx_ringbuffer_write_pos - rx_ringbuffer_read_pos)<0) messages_to_process = SPI_RX_RINGBUFFERSIZE + rx_ringbuffer_write_pos - rx_ringbuffer_read_pos;
  else messages_to_process = rx_ringbuffer_write_pos - rx_ringbuffer_read_pos;
}

// SPI-Transmission - wait for flag set in interrupt routine
void spi_buffer_handling() {
  //if (process_it){
  if (messages_to_process>0){
    //rx_buf [pos] = 0; //set end of string

    #ifdef DEBUG_PROGRAM_FLOW_ON
      Serial.println("1");
    #endif

    //debug
    #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_SPI_ON
        Serial.print("Messages to process: ");
        Serial.print(messages_to_process, DEC);
        Serial.print(", rx_ringbuffer_read_pos: ");
        Serial.print(rx_ringbuffer_read_pos, DEC);
        Serial.print(", rx_ringbuffer_write_pos: ");
        Serial.println(rx_ringbuffer_write_pos, DEC);
    #endif
    
    //Debug
    //String debug_string;
    #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_SPI_ON
      //#error SPI debug-msg compilation activated!
        Serial.print("SPI-Buffer:");
        for (int i=0; i<pos; i++) {
          Serial.print(" ");
          //debug_string =  String(rx_doublebuf[rx_ringbuffer_read_pos][i], DEC);
          //Serial.print(debug_string);
          Serial.print(rx_doublebuf[rx_ringbuffer_read_pos][i], HEX);
        }
        Serial.println();
    #endif

    process_incomming_msg();

    //Update rx_ringbuffer_read_pos
    if (rx_ringbuffer_read_pos<SPI_RX_RINGBUFFERSIZE-1) rx_ringbuffer_read_pos++;
    else rx_ringbuffer_read_pos = 0;

    update_messages_to_process();

    //debug
    #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_SPI_ON
        Serial.print("Messages to process: ");
        Serial.print(messages_to_process, DEC);
        Serial.print(", rx_ringbuffer_read_pos: ");
        Serial.print(rx_ringbuffer_read_pos, DEC);
        Serial.print(", rx_ringbuffer_write_pos: ");
        Serial.println(rx_ringbuffer_write_pos, DEC);
    #endif
    
    create_machine_state_msg();

    byte_received = false; //reset flag
    //process_it = false; //reset flag
  }
} //end of receive_spi

unsigned char CRC8 (volatile unsigned char * buf, unsigned char message_offset, unsigned char used_message_bytes, boolean verify_with_extra_byte, unsigned char msg_crc_8) {
  //ATTENTION: if all bytes are zero, CRC8-Check is always correct!!! High possibility of incorrect detected message.
  //Set verify_with_extra_byte=false and msg_crc_8=0 (or whatever) to get the crc_8-value of the msg returned
  //If the last byte of the message is the correct crc-value of the bytes before, CRC8 returns 0.
  //Set verify_with_extra_byte=true and msg_crc_8=crc_8 of the message (byte 17) to verify the used message bytes against it. 
  //used_message_bytes is needed, because the crc_8 is only transmitted for used bytes.
  unsigned char bytecount, data, crc_8=0;
  
  for (bytecount=message_offset; bytecount<(used_message_bytes+message_offset); bytecount++) {
    data = (unsigned char) buf[bytecount];
    crc_8 = _crc8_ccitt_update (crc_8,data);
  }
  if (verify_with_extra_byte) crc_8 = _crc8_ccitt_update (crc_8,msg_crc_8);
  
  return crc_8;
}

boolean check_msg(char msg_length, boolean force_action) {
  //debug lastsuccessful_msg
  #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_SPI_ON
    Serial.print("lastsuccessful_msg DEC: ");
    Serial.print(lastsuccessful_msg, DEC);
    Serial.print(", lastsuccessful_msg HEX: ");
    Serial.print(lastsuccessful_msg, HEX);
    Serial.print(", actual_msg HEX: ");
    Serial.println(rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_NO], HEX);
  #endif
  boolean success = true;
  if (CRC8(rx_doublebuf[rx_ringbuffer_read_pos], 0, msg_length, true, rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_CRC8])) {
    success=false; //msg-failure
    set_error(ERROR_SPI_BIT);
  }
  else if (rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_NO] == (lastsuccessful_msg+1)%256) { //no message lost
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
  switch(rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_TYPE]) {
    case 1:   //Update Machine State
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              break;
    case 2:   //Programm Start at Block
              msg_length=4;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && initialized && ((STATE>>STATE_PAUSE_BIT)&1) && command_completed) {
                programm_start((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_N_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_N_L]);
              }
              break;
    case 3:   //Programm Stop
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else programm_stop();
              break;
    case 4:   //Programm Pause
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else programm_pause();
              break;
    case 5:   //Spindle on with RPM and Direction
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && initialized && ((STATE>>STATE_MANUAL_BIT)&1)) {
                set_revolutions((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_RPM_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_RPM_L]);
                spindle_direction(rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_DIRECTION]);
                spindle_on();
              }
              break;
    case 6:   //Spindle off
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                spindle_off();
              }
              break;
    case 7:   //Stepper on
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && initialized && ((STATE>>STATE_MANUAL_BIT)&1)) {
                if (command_completed) { //Error Handling needed
                  stepper_on();
                }
              }
              break;
    case 8:   //Stepper off
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else if ((STATE>>STATE_MANUAL_BIT)&1) {
                if (command_completed) { //Error Handling needed
                  stepper_off();
                }
              }
              break;
    case 9:   //X-Stepper move with feed
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && initialized && ((STATE>>STATE_MANUAL_BIT)&1) && command_completed) {
                set_xz_stepper_manual((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_F_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_F_L], rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_DIRECTION], 0);
              }
              break;
    case 10:   //Z-Stepper move with feed
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && initialized && ((STATE>>STATE_MANUAL_BIT)&1) && command_completed) {
                set_xz_stepper_manual((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_F_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_F_L], rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_DIRECTION], 1);
              }
              break;
    case 11:   //Set Tool-Position (and INIT)
              msg_length=7;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && ((STATE>>STATE_MANUAL_BIT)&1) && command_completed) {
                get_Tool_X((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_X_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_X_L]);
                get_Tool_Z((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_TOOL_Z_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_TOOL_Z_L]);
                set_tool_position(rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_T]);
              }
              break;
    case 12:   //Origin-XOffset
              msg_length=4;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && initialized && ((STATE>>STATE_MANUAL_BIT)&1) && command_completed) { //Error Handling needed
                set_x_coordinate((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_X_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_X_L]);
              }
              break;
    case 13:   //Origin-ZOffset
              msg_length=4;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && initialized && ((STATE>>STATE_MANUAL_BIT)&1) && command_completed) { //Error Handling needed
                /*
                #ifndef DEBUG_SERIAL_CODE_OFF
                  Serial.print("Z DEC: ");
                  Serial.print(((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_L]), DEC);
                  Serial.print(", ZH HEX: ");
                  Serial.print((rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_H]), HEX);
                  Serial.print(", ZL HEX: ");
                  Serial.print((rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_L]), HEX);
                  Serial.print(", Z HEX: ");
                  Serial.println(((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_L]), HEX);
                #endif
                */
                set_z_coordinate((((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_Z_L]);
              }
              break;
    case 14:  //metric or inch (maybe not needed)
              msg_length=3;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if (get_control_active() && ((STATE>>STATE_MANUAL_BIT)&1) && command_completed) { //Error Handling needed
                if (rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_INCH]){
                  STATE |= _BV(STATE_INCH_BIT); //set STATE_bit4 = 1
                }
                else STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
              }
              break;
    case 15:  //New CNC-Programm wit N Blocks in metric or inch
              msg_length=5;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                //some Error-Handling needed, if message is ignored
                programm_stop();
                //int NMAX = ((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_N_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_N_L]) //maybe not needed
                for (N=0; N<CNC_CODE_NMAX; N++) {
                  cnc_code[N].GM = 0;
                }
                //metric or inch? maybe better replaced by G-Code
                if (rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_G_INCH]){
                  STATE |= _BV(STATE_INCH_BIT); //set STATE_bit4 = 1
                }
                else STATE &= ~(_BV(STATE_INCH_BIT)); //set STATE_bit4 = 0
              }
              else success=1;
              break;
    case 16:  //CNC-Code-Block
              msg_length=14;
              if (!check_msg(msg_length, false)) success=false; //msg-failure
              else if ((STATE>>STATE_PAUSE_BIT)&1) {
                //some Error-Handling needed, if message is ignored
                N = (((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_N_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_N_L];
                cnc_code[N].GM = rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_GM];
                cnc_code[N].GM_NO = rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_GM_NO];
                cnc_code[N].XI = (((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_XI_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_XI_L];
                cnc_code[N].ZK = (((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_ZK_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_ZK_L];
                cnc_code[N].FTLK = (((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_FTLK_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_FTLK_L];
                cnc_code[N].HS = (((int)rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_HS_H])<<8) | rx_doublebuf[rx_ringbuffer_read_pos][SPI_BYTE_RASPI_MSG_HS_L];
              }
              else success=1;
              break;
    case 17:  //Shutdown
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else {
                programm_stop();
                stepper_off();
                spindle_off();
                save_current_x_step();
                save_current_z_step();
                save_current_x_coordinate();
                save_current_z_coordinate();
                save_current_tool_position();
              }
              break;
    case 18:  //Load last coordinates and tool position and init
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else if (get_control_active() && ((STATE>>STATE_MANUAL_BIT)&1) && command_completed) {
                read_last_x_step();
                read_last_z_step();
                read_last_x_coordinate();
                read_last_z_coordinate();
                read_current_tool_position();
                //initialize
                STATE |= _BV(STATE_INIT_BIT); //set STATE_bit1 = STATE_INIT
                initialized=1;
              }
              break;
    case 19:  //Reset Errors
              if (!check_msg(msg_length, true)) success=false; //msg-failure
              else {
                ERROR_NO = 0;
              }
              break;
    default:  //SPI-Error "PID unkown"
              success=false;
  }
  return success;
}

void init_msg_praeambel() {
  tx_buf [0] = 0x7F;
  tx_buf [1] = 0xFF;
  tx_buf [2] = 0x7F;
  tx_buf [3] = 0xFF;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { //needed? 8bit?
    SPDR = tx_buf [0]; //first byte for sending at next interrupt
  }
}

void create_machine_state_msg() {
  //maybe the whole function needs to be atomic or tx double buffered!!!
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_TYPE] = 100; //PID
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_LASTSUCCESS_MSG_NO] = lastsuccessful_msg;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_STATE] = STATE; //bit6_stepper|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_RPM_H] = STATE_RPM>>8;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_RPM_L] = STATE_RPM;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_X_H] = STATE_X>>8;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_X_L] = STATE_X;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_Z_H] = STATE_Z>>8;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_Z_L] = STATE_Z;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_F_H] = STATE_F>>8;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_F_L] = STATE_F;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_H_H] = STATE_H>>8;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_H_L] = STATE_H;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_T] = STATE_T; //0 = uninitialized
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_N_H] = STATE_N>>8;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_N_L] = STATE_N;
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_ERROR_NO] = ERROR_NO; //bit2_SPINDLE|bit1_CNC_CODE|bit0_SPI
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_CRC8] = CRC8(tx_doublebuf, 0, SPI_MSG_LENGTH-SPI_BYTE_LENGTH_PRAEAMBEL-1, false, 0);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    //copy tx_buf to tx_doublebuf without praeambel
    for (int i=0; i<SPI_MSG_LENGTH-SPI_BYTE_LENGTH_PRAEAMBEL; i++) {
      tx_buf[i+SPI_BYTE_LENGTH_PRAEAMBEL] = tx_doublebuf[i];
    }
  }
}

void create_other_msg() { //maybee not needed
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_TYPE] = 101; //PID
  tx_doublebuf [SPI_BYTE_ARDUINO_MSG_LASTSUCCESS_MSG_NO] = lastsuccessful_msg;
  for (int i=2; i<(SPI_MSG_LENGTH-SPI_BYTE_LENGTH_PRAEAMBEL-2); i++) {
    tx_doublebuf [i] = 0;
  }
  tx_doublebuf [SPI_MSG_LENGTH-1] = CRC8(tx_doublebuf, 0, 2, false, 0);
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    for (int i=0; i<SPI_MSG_LENGTH-SPI_BYTE_LENGTH_PRAEAMBEL; i++) {
      tx_buf[i+SPI_BYTE_LENGTH_PRAEAMBEL] = tx_doublebuf[i];
    }
  }
}

// SPI interrupt routine
ISR (SPI_STC_vect) {
  byte c = SPDR;  // grab byte from SPI Data Register
  byte_received = true;
  
  // add to rx_buffer if room and write next tx_buffer-value in SPDR-Register
  if (pos < sizeof(rx_buf)) {
    rx_buf [pos] = c; //received byte

    //if Praeambel not detected shift pos back
    if (pos == SPI_BYTE_LENGTH_PRAEAMBEL-1) {
      if (rx_buf[0] != 0x7F && rx_buf[1] != 0xFF && rx_buf[2] != 0x7F && rx_buf[3] != 0xFF) {
        rx_buf[0] = rx_buf[1];
        rx_buf[1] = rx_buf[2];
        rx_buf[2] = rx_buf[3];
        pos--;
      }
    }
    
    // if Praeambel detected later reset pos
    if (pos > SPI_BYTE_LENGTH_PRAEAMBEL) {
      if (rx_buf[pos-SPI_BYTE_LENGTH_PRAEAMBEL] == 0x7F && rx_buf[pos-SPI_BYTE_LENGTH_PRAEAMBEL+1] == 0xFF && rx_buf[pos-SPI_BYTE_LENGTH_PRAEAMBEL+2] == 0x7F && rx_buf[pos-SPI_BYTE_LENGTH_PRAEAMBEL+3] == 0xFF) {
        rx_buf[0] = 0x7F;
        rx_buf[1] = 0xFF;
        rx_buf[2] = 0x7F;
        rx_buf[3] = 0xFF;
        rx_buf[SPI_BYTE_LENGTH_PRAEAMBEL] = rx_buf [pos];
        pos = SPI_BYTE_LENGTH_PRAEAMBEL;
      }
    }

    /*
    // newline means time to process buffer
    if (c == '\n') {
      process_it = true;
    }
    */
  }  // end of room available

  /*
  //for debugging
  #if !defined DEBUG_SERIAL_CODE_OFF && defined DEBUG_MSG_SPI_ON
    //#error SPI debug-msg compilation activated!
    Serial.print("pos: ");
    Serial.println(pos);
  #endif
  */

  pos++;
  
  if (pos < sizeof(tx_buf)) {
    SPDR = tx_buf [pos]; //byte for sending at next interrupt
  }
  
  //process buffer at fixed SPI_MSG_LENGTH
  if (pos == SPI_MSG_LENGTH) {
    if (messages_to_process<SPI_RX_RINGBUFFERSIZE) {
      //copy rx_buf to rx_doublebuf[rx_ringbuffer_write_pos] without praeambel
      for (int i=SPI_BYTE_LENGTH_PRAEAMBEL; i<SPI_MSG_LENGTH; i++) {
        rx_doublebuf[rx_ringbuffer_write_pos][i-SPI_BYTE_LENGTH_PRAEAMBEL] = rx_buf[i];
      }
      //Update rx_ringbuffer_write_pos
      if (rx_ringbuffer_write_pos<SPI_RX_RINGBUFFERSIZE-1) rx_ringbuffer_write_pos++;
      else rx_ringbuffer_write_pos = 0;
      update_messages_to_process();
    }
    else set_error(ERROR_SPI_BIT);
    pos = 0;
    SPDR = tx_buf [0]; //first byte for sending at next interrupt
    //process_it = true;
  }
  
}  // end of interrupt routine SPI_STC_vect

