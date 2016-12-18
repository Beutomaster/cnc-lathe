#ifndef Raspi_SPI_h
#define Raspi_SPI_h

//includes
#include <util/atomic.h>
#include <util/crc16.h>
#include <Arduino.h>
#include "CNC_Lathe.h"
#include "Motion_Control.h"
#include "CNC_Control.h"
#include "Spindle_Control.h"
#include "Tool_Changer_Control.h"
#include <SPI.h>

//defines
#define SPI_MSG_LENGTH 18

//Byte Postions of Arduino-Answer
#define SPI_BYTE_PID 0
#define SPI_BYTE_LASTSUCCESS_MSG_NO 1
#define SPI_BYTE_STATE 2
#define SPI_BYTE_RPM_H 3
#define SPI_BYTE_RPM_L 4
#define SPI_BYTE_X_H 5
#define SPI_BYTE_X_L 6
#define SPI_BYTE_Z_H 7
#define SPI_BYTE_Z_L 8
#define SPI_BYTE_F_H 9
#define SPI_BYTE_F_L 10
#define SPI_BYTE_H_H 11
#define SPI_BYTE_H_L 12
#define SPI_BYTE_T 13
#define SPI_BYTE_N_H 14
#define SPI_BYTE_N_L 15
#define SPI_BYTE_ERROR_NO 16
#define SPI_BYTE_CRC8 17

//Byte Postions of RASPI-Msg
#define SPI_BYTE_RASPI_MSG_NO 1
#define SPI_BYTE_RASPI_MSG_N_H 2
#define SPI_BYTE_RASPI_MSG_N_L 3
#define SPI_BYTE_RASPI_MSG_GM 4
#define SPI_BYTE_RASPI_MSG_GM_NO 5
#define SPI_BYTE_RASPI_MSG_XI_H 6
#define SPI_BYTE_RASPI_MSG_XI_L 7
#define SPI_BYTE_RASPI_MSG_ZK_H 8
#define SPI_BYTE_RASPI_MSG_ZK_L 9
#define SPI_BYTE_RASPI_MSG_FTLK_H 10
#define SPI_BYTE_RASPI_MSG_FTLK_L 11
#define SPI_BYTE_RASPI_MSG_HS_H 12
#define SPI_BYTE_RASPI_MSG_HS_L 13
#define SPI_BYTE_RASPI_MSG_RPM_H 2
#define SPI_BYTE_RASPI_MSG_RPM_L 3
#define SPI_BYTE_RASPI_MSG_DIRECTION 4
#define SPI_BYTE_RASPI_MSG_F_H 2
#define SPI_BYTE_RASPI_MSG_F_L 3
#define SPI_BYTE_RASPI_MSG_X_H 2
#define SPI_BYTE_RASPI_MSG_X_L 3
#define SPI_BYTE_RASPI_MSG_Z_H 4
#define SPI_BYTE_RASPI_MSG_Z_L 5
#define SPI_BYTE_RASPI_MSG_T 6
#define SPI_BYTE_RASPI_MSG_INCH 2
#define SPI_BYTE_RASPI_MSG_G_INCH 4

extern volatile boolean byte_received;
extern char tx_buf [SPI_MSG_LENGTH]; //SPI send-buffer

//Controlled by (Timer-) Interrupts:
void spi_buffer_handling();
unsigned char CRC8 (char *, unsigned char, boolean, unsigned char);
boolean process_incomming_msg();
void create_machine_state_msg();
void create_other_msg();
//ISR (SPI_STC_vect);

#endif

