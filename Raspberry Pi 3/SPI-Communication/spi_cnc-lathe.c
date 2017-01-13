/* spi_cnc-lathe.c
 * SPI Communication Tool for an modified Emco Compact 5 CNC-Lathe
 * including a pipe-Server (for a php-script of a Webinterface)
 * test with second console: echo "test" >> /home/pi/spi_com/arduino_pipe.tx
 * written by Hannes Beuter
 * 
 * influenced by following Code Examples:
 * --------------------------------------
 * SPI testing utility (using spidev driver)
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 * On Raspberry Pi compile with: gcc -o spi_cnc-lathe spi_cnc-lathe.c
 */
 
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
//#include <sys/types.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <signal.h>
#include <sys/select.h>
//#include <sys/time.h>
//#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

//defines

//Pipe-Server
#define COMMAND_PIPE "/home/pi/spi_com/arduino_pipe.tx"
#define SPI_TX_RINGBUFFERSIZE 500
#define BUF 100
//#define BUF 4096
#define MSG_SUCCESS "Sending your message to Arduino!\0"
#define MSG_ERROR_STATE "Could not send message to Arduino at this Machine State!\0"
#define MSG_ERROR_CLIENT "Could not send message to Arduino, because it is in exclusively use by another Client!\0"
//#define CLIENT_SESSION_ID_ARRAY_LENGTH 27
#define CLIENT_SESSION_ID_ARRAY_LENGTH 100
//example-sid: 15a1rgdq662cms5m9qe3f55n74

//SPI
#define SPI_BYTE_LENGTH_PRAEAMBEL 4
#define SPI_MSG_LENGTH (18+SPI_BYTE_LENGTH_PRAEAMBEL)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

//Input Parameter Ranges
#define CNC_CODE_NMIN 0
#define CNC_CODE_NMAX 500
#define GM_CODE_MIN 0
#define G_CODE_MAX 196
#define M_CODE_MAX 99
#define X_MIN_MAX_CNC 5999
#define Z_MIN_MAX_CNC 32760
#define XZ_MIN_MAX_HAND 89999L
#define X_DWELL_MIN_MAX_CNC 5999
#define F_MIN 2
#define F_MAX 499
#define IK_MIN 0
#define I_MAX 5999
#define K_MAX 5999
#define K_THREAD_PITCH_MAX 499
#define L_MIN 0
#define L_MAX 499
#define T_MIN 1
#define T_MAX 6
#define H_MIN 0
#define H_G86_MIN 10
#define H_MAX 999
#define REVOLUTIONS_MIN 460  //rpm
#define REVOLUTIONS_MAX 3220 //rpm

//Byte Postions of Arduino-Answer
#define SPI_BYTE_ARDUINO_MSG_TYPE (0+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_MSG_LASTSUCCESS_MSG_NO (1+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_MSG_STATE (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_RPM_H (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_RPM_L (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_X_H (5+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_X_L (6+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_Z_H (7+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_Z_L (8+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_F_H (9+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_F_L (10+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_H_H (11+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_H_L (12+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_T (13+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_N_H (14+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_N_L (15+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_ERROR_NO (16+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_CRC8 (17+SPI_BYTE_LENGTH_PRAEAMBEL)

//Byte Postions of RASPI-Msg
#define SPI_BYTE_RASPI_MSG_TYPE (0+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_NO (1+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_N_H (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_N_L (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_N_OFFSET_H (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_N_OFFSET_L (5+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_GM (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_GM_NO (5+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_XI_H (6+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_XI_L (7+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_ZK_H (8+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_ZK_L (9+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_FTLK_H (10+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_FTLK_L (11+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_HS_H (12+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_HS_L (13+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_RPM_H (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_RPM_L (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_DIRECTION (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_F_H (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_F_L (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_X_H (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_X_L (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_Z_H 2 (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_Z_L 3 (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_TOOL_Z_H (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_TOOL_Z_L (5+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_T (6+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_INCH (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_G_INCH (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_CRC8 (17+SPI_BYTE_LENGTH_PRAEAMBEL)

//Bit Postions of STATE
#define STATE_CONTROL_ACTIVE_BIT 0
#define STATE_INIT_BIT 1
#define STATE_MANUAL_BIT 2
#define STATE_PAUSE_BIT 3
#define STATE_INCH_BIT 4
#define STATE_SPINDLE_BIT 5
#define STATE_SPINDLE_DIRECTION_BIT 6
#define STATE_STEPPER_BIT 7

//Bit Postions of ERROR_NO (actual ERROR-Numbers Bit-coded)
#define ERROR_SPI_BIT 0
#define ERROR_CNC_CODE_BIT 1
#define ERROR_SPINDLE_BIT 2
#define ERROR_CNC_CODE_NEEDED_BIT 3

#define MACHINE_STATE_FILE "/var/www/html/xml/machine_state.xml"
//#define MACHINE_STATE_FILE "~/machine_state.xml" //does not work
//#define MACHINE_STATE_FILE "machine_state.xml"

//global vars
//Machine-State
uint8_t STATE_T=0, STATE_active=0, STATE_init=0, STATE_manual=0, STATE_pause=0, STATE_inch=0, STATE_spindle_on=0, STATE_spindle_direction=0, STATE_stepper_on=0, ERROR_spi_error=0, ERROR_cnc_code_error=0, ERROR_spindle_error=0;
int16_t STATE_N=0, STATE_RPM=0,  STATE_X=0, STATE_Z=0, STATE_F=0, STATE_H=0;

//spi-Master
FILE *machinestatefile;
int spi_fd;
uint8_t msg_number=1, lastsuccessful_msg =0;
//uint8_t tx[SPI_MSG_LENGTH] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0};
uint8_t tx[SPI_MSG_LENGTH] = {0x7F,0xFF,0x7F,0xFF, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0};

//pipe-Server
char start_pipe_server=1, verbose = 1, state=0, client_sid[CLIENT_SESSION_ID_ARRAY_LENGTH], exclusive[CLIENT_SESSION_ID_ARRAY_LENGTH], buffer[SPI_TX_RINGBUFFERSIZE][BUF], answer_to_client[BUF], answer_fifo_name[BUF];
int r_fd, w_fd, i, ret, ringbuffer_pos=0, messages_notreceived=0, ringbuffer_fill_status=0;
//FILE *r_fz, *w_fz;
//parameter for select
fd_set r_fd_set;
struct timeval timeout;

//File-Parser
char process_file = 0;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum)
{
	printf("\nCaught signal %d\n",signum);
	// Cleanup and close up stuff here
	printf("close(spi_fd)\n");
	close(spi_fd);
	printf("close(r_fd)\n");
	close(r_fd);
	//printf("fclose(machinestatefile)\n");
	//fclose(machinestatefile)
		
	// Terminate program
	exit(signum);
}

setup_pipe_server() {
	//Server creates arduino_pipe.tx, if it does not exist
	if (mkfifo (COMMAND_PIPE, O_RDWR | 0666) < 0) {
	  //arduino_pipe.tx exists
	  if(errno == EEXIST)
		 printf ("Backend Pipe-Server: arduino_pipe.tx exists, trying to use it!\n");
	  else {
		 perror("Backend Pipe-Server: mkfifio()");
		 exit (EXIT_FAILURE);
	  }
	}

	//Server opens arduino_pipe.tx readonly
	/*r_fd = open ("arduino_pipe.tx", O_RDONLY);
	You opened that FIFO as read only (O_RDONLY), whenever there is no writer to the FIFO, the read end will receive an EOF.
	Select system call will return on EOF and for every EOF you handle there will be a new EOF. This is the reason for the observed behavior.
	To avoid this open that FIFO for both reading and writing (O_RDWR). This ensures that you have at least one writer on the FIFO thus there wont be an EOF and as a result select won't return unless someone writes to that FIFO.
	*/
	//Server opens arduino_pipe.tx
	r_fd = open (COMMAND_PIPE, O_RDWR); //create an Filedescriptor for Low-Level I/O-Functions like read/write
	if (r_fd == -1) {
	  perror("Backend File-Parser: open(1)");
	  exit (EXIT_FAILURE);
	}
	//r_fz = fdopen(r_fd, "r"); //create an Filepointer for High-Level I/O-Functions like fscanf
}

/*The SPI-Driver supports following speeds:

  cdiv     speed          cdiv     speed
    2    125.0 MHz          4     62.5 MHz
    8     31.2 MHz         16     15.6 MHz
   32      7.8 MHz         64      3.9 MHz
  128     1953 kHz        256      976 kHz
  512      488 kHz       1024      244 kHz
 2048      122 kHz       4096       61 kHz
 8192     30.5 kHz      16384     15.2 kHz
32768     7629 Hz
*/

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 122000; //max speed in Hz (at 500000 Hz the Arduino receives not all bytes for sure)
static uint16_t delay;

int setup_spi()
{
	int ret = 0;

	spi_fd = open(device, O_RDWR);
	if (spi_fd < 0)
		pabort("can't open SPI-device");
	
	// Register signal and signal handler
	signal(SIGINT, signal_callback_handler);

	/*
	 * spi mode
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(spi_fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(spi_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(spi_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return ret;
}

uint8_t _crc8_ccitt_update (uint8_t inCrc, uint8_t inData) {
	uint8_t i;
	uint8_t data;

	data = inCrc ^ inData;

	for ( i = 0; i < 8; i++ ) {
		if (( data & 0x80 ) != 0 ) {
			data <<= 1;
			data ^= 0x07;
		}
		else {
			data <<= 1;
		}
	}
	return data;
}

uint8_t CRC8 (uint8_t * buf, uint8_t message_offset, uint8_t used_message_bytes) {
  //get the crc_8-value of the msg returned
  //If the last byte of the message is the correct crc-value of the bytes before, CRC8 returns 0.
  uint8_t bytecount, data, crc_8=0;
  
  for (bytecount=message_offset;bytecount<(used_message_bytes+message_offset);bytecount++) {
    data = buf[bytecount];
    crc_8 = _crc8_ccitt_update (crc_8,data);
  }
  
  return crc_8;
}

static int spi_create_command_msg(const char *pipe_msg_buffer, char msg_type) {
	int n, block=-1, rpm=-1, spindle_direction=-1, negativ_direction=-1, XX=32767, ZZ=32767, feed=-1, tool=0, inch=-1, gmcode=-1, HH=-1, code_type=0;
	uint8_t used_length=0, pos=SPI_BYTE_LENGTH_PRAEAMBEL;
	
	const char *pipe_msg_buffer_temp = pipe_msg_buffer; //hotfix
	
	/*
	//debug
	printf("pipe_msg_buffer %p, length %i\n", pipe_msg_buffer, sizeof(pipe_msg_buffer));
	printf("client_sid %p, length %i\n", client_sid, sizeof(client_sid));
	printf("msg_type %p, length %i\n", &msg_type, sizeof(msg_type));
	*/
	
	if (!msg_type) {
		if (pipe_msg_buffer == NULL) { //command-line-mode
			//User Input for Message
			printf("Message-Types:\n");
			printf("--------------\n");
			printf("001 Update Machine State\n");
			printf("002 Programm Start at Block\n");
			printf("003 Programm Stop\n");
			printf("004 Programm Pause\n");
			printf("005 Spindle on with RPM and Direction\n");
			printf("006 Spindle off\n");
			printf("007 Stepper on\n");
			printf("008 Stepper off\n");
			printf("009 X-Stepper move with feed\n");
			printf("010 Z-Stepper move with feed\n");
			printf("011 Set Tool-Position (and INIT)\n");
			printf("012 Origin-X-Offset\n");
			printf("013 Origin-Z-Offset\n");
			printf("014 metric or inch (maybe not needed)\n");
			printf("015 New CNC-Programm wit NN Blocks in metric or inch\n");
			printf("016 CNC-Code-Block\n");
			printf("017 shutdown\n");
			printf("018 Load last coordinates and tool position and init\n");
			printf("019 Reset Errors\n\n");
		}
		else {
			n = sscanf(pipe_msg_buffer,"%s %d", client_sid, &msg_type);
			if (n != 2) {
				if (errno != 0) perror("Backend Command-Interpreter: scanf");
				else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
				return 0;
			}
			//printf ("n: %i\n", n);
			pipe_msg_buffer = pipe_msg_buffer_temp;
			printf ("Message from Client-SESSION: %s\n", client_sid);
		}
	}
	
	/*
	//debug
	printf("pipe_msg_buffer %p:\n", pipe_msg_buffer);
	printf("client_sid %p, length %i\n", client_sid, sizeof(client_sid));
	printf("msg_type %p, length %i\n", &msg_type, sizeof(msg_type));
	*/
	printf("Message-Type: ");
	if ((msg_type>=1) && (msg_type<=19)) printf("%i\n",msg_type);
	else if (pipe_msg_buffer == NULL) {
		do {
			scanf("%d",&msg_type);
			getchar();
		} while ((msg_type<1) || (msg_type>19));
	}
	else {
		fprintf(stderr, "Backend Command-Interpreter: Command out of Range\n");
		return(EXIT_FAILURE);
	}
	tx[pos++] = msg_type;
		
	//Message-Number
	printf("Message-No: %i\n\n", msg_number);
	tx[pos++] = msg_number;
	
	switch (msg_type) {
		case 1:   	//Update Machine State
					break;
		case 2:   	//Programm Start at Block
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d", client_sid, &msg_type, &block);
						if (n != 3) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("Block-No (0 to 500): ");
					if ((block>=CNC_CODE_NMIN) && (block<=CNC_CODE_NMAX)) printf("%i\n",block);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&block);
							getchar();
						} while ((block<CNC_CODE_NMIN) || (block>CNC_CODE_NMAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: N out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = block>>8;
					tx[pos++] = block;
					break;
		case 3: 	//Programm Stop
					break;
		case 4:   	//Programm Pause
					break;
		case 5:   	//Spindle on with RPM and Direction
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d %d", client_sid, &msg_type, &rpm, &spindle_direction);
						if (n != 4) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("RPM (460 to 3220): ");
					if ((rpm>=REVOLUTIONS_MIN) && (rpm<=REVOLUTIONS_MAX)) printf("%i\n",rpm);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&rpm);
							getchar();
						} while ((rpm<REVOLUTIONS_MIN) || (rpm>REVOLUTIONS_MAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: rpm out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = rpm>>8;
					tx[pos++] = rpm;
					
					printf("Spindle direction invers (0 or 1): ");
					if ((spindle_direction>=0) && (spindle_direction<=1)) printf("%i\n",spindle_direction);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&spindle_direction);
							getchar();
						} while ((spindle_direction<0) || (spindle_direction>1));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: spindle-direction out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = spindle_direction;
					break;
		case 6:  	//Spindle off
					break;
		case 7:   	//Stepper on
					break;
		case 8:   	//Stepper off
					break;
		case 9:   	//X-Stepper move with feed
		case 10:   	//Z-Stepper move with feed
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d", client_sid, &msg_type, &feed, &negativ_direction);
						if (n != 4) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("Feed (2 to 499): ");
					if ((feed>=F_MIN) && (feed<=F_MAX)) printf("%i\n",feed);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&feed);
							getchar();
						} while ((feed<F_MIN) || (feed>F_MAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: F out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = feed>>8;
					tx[pos++] = feed;
					
					printf("negativ direction (0 or 1): ");
					if ((negativ_direction>=0) && (negativ_direction<=1)) printf("%i\n",negativ_direction);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&negativ_direction);
							getchar();
						} while ((negativ_direction<0) || (negativ_direction>1));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: Stepper direction out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = negativ_direction;
					break;
		case 11:   	//Set Tool-Position (and INIT)
					//debug
					//printf("pipe_msg_buffer %p:\n", pipe_msg_buffer);
					if (pipe_msg_buffer != NULL) {
						printf("Parameter einlesen\n");
						n = sscanf(pipe_msg_buffer,"%s %d %d %d %d", client_sid, &msg_type, &XX, &ZZ, &tool);
						if (n != 5) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("X-Offset (+-5999): ");
					if ((XX>=-X_MIN_MAX_CNC) && (XX<=X_MIN_MAX_CNC)) printf("%i\n",XX);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&XX);
							getchar();
						} while ((XX<-X_MIN_MAX_CNC) || (XX>X_MIN_MAX_CNC));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: X out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = XX>>8;
					tx[pos++] = XX;
					
					printf("Z-Offset (+-32700): ");
					if ((ZZ>=-Z_MIN_MAX_CNC) && (ZZ<=Z_MIN_MAX_CNC)) printf("%i\n",ZZ);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&ZZ);
							getchar();
						} while ((ZZ<-Z_MIN_MAX_CNC) || (ZZ>Z_MIN_MAX_CNC));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: Z out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = ZZ>>8;
					tx[pos++] = ZZ;
					
					printf("Tool (1 to 6): ");
					if ((tool>=T_MIN) && (tool<=T_MAX)) printf("%i\n",tool);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&tool);
							getchar();
						} while ((tool<T_MIN) || (tool>T_MAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: T out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = tool;
					break;
		case 12:   	//Origin-X-Offset
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d", client_sid, &msg_type, &XX);
						if (n != 3) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("X-Offset (+-5999): ");
					if ((XX>=-X_MIN_MAX_CNC) && (XX<=X_MIN_MAX_CNC)) printf("%i\n",XX);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&XX);
							getchar();
						} while ((XX<-X_MIN_MAX_CNC) || (XX>X_MIN_MAX_CNC));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: X out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = XX>>8;
					tx[pos++] = XX;
					break;
		case 13:   	//Origin-Z-Offset
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d", client_sid, &msg_type, &ZZ);
						if (n != 3) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("Z-Offset (+-32760): ");
					if ((ZZ>=-Z_MIN_MAX_CNC) && (ZZ<=Z_MIN_MAX_CNC)) printf("%i\n",ZZ);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&ZZ);
							getchar();
						} while ((ZZ<-Z_MIN_MAX_CNC) || (ZZ>Z_MIN_MAX_CNC));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: Z out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = ZZ>>8;
					tx[pos++] = ZZ;
					break;
		case 14:  	//metric or inch (maybe not needed)
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d", client_sid, &msg_type, &inch);
						if (n != 3) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("metric or inch (0 or 1): ");
					if ((inch>=0) && (inch<=1)) printf("%i\n",inch);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&inch);
							getchar();
						} while ((inch<0) || (inch>1));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: metric or inch out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = inch;
					break;
		case 15:  	//New CNC-Programm wit N Blocks in metric or inch
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d %d", client_sid, &msg_type, &block, &inch);
						if (n != 4) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("Blocks (0 to 500): ");
					if ((block>=CNC_CODE_NMIN) && (block<=CNC_CODE_NMAX)) printf("%i\n",block);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&block);
							getchar();
						} while ((block<CNC_CODE_NMIN) || (block>CNC_CODE_NMAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: N out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = block>>8;
					tx[pos++] = block;
					
					printf("metric or inch (0 or 1): ");
					if ((inch>=0) && (inch<=1)) printf("%i\n",inch);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&inch);
							getchar();
						} while ((inch<0) || (inch>1));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: metric or inch out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = inch;
					
					process_file = 1;
					break;
		case 16:  	//CNC-Code-Block
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %c %d %d %d %d %d", client_sid, &msg_type, &block, &code_type, &gmcode, &XX, &ZZ, &feed, &HH);
						if (n != 9) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return 0;
						}
					}
					
					printf("Block-No (0 to 500): ");
					if ((block>=CNC_CODE_NMIN) && (block<=CNC_CODE_NMAX)) printf("%i\n",block);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&block);
							getchar();
						} while ((block<CNC_CODE_NMIN) || (block>CNC_CODE_NMAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: N out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = block>>8;
					tx[pos++] = block;
					
					printf("G- or M-Code (G or M): ");
					if ((code_type == 'G') || (code_type == 'M')) printf("%c\n",code_type);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%c",&code_type);
							getchar();
						} while ((code_type != 'G') && (code_type != 'M'));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: G or M out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = code_type;
					
					if (code_type == 'G') {
						printf("G-Code (0 to 196): "); //not a real check
						if ((gmcode>=GM_CODE_MIN) && (gmcode<=G_CODE_MAX)) printf("%i\n",gmcode);
						else if (pipe_msg_buffer == NULL) {
							do {
								scanf("%d",&gmcode);
								getchar();
							} while ((gmcode<GM_CODE_MIN) || (gmcode>G_CODE_MAX));
						}
						else {
							fprintf(stderr, "Backend Command-Interpreter: G- or M-Code out of Range\n");
							return(EXIT_FAILURE);
						}
					}
					else {
						printf("M-Code (0 to 99): "); //not a real check
						if ((gmcode>=GM_CODE_MIN) && (gmcode<=M_CODE_MAX)) printf("%i\n",gmcode);
						else if (pipe_msg_buffer == NULL) {
							do {
								scanf("%d",&gmcode);
								getchar();
							} while ((gmcode<GM_CODE_MIN) || (gmcode>M_CODE_MAX));
						}
						else {
							fprintf(stderr, "Backend Command-Interpreter: G- or M-Code out of Range\n");
							return(EXIT_FAILURE);
						}
					}
					tx[pos++] = gmcode;
					
					printf("X (+-5999): ");
					if ((XX>=-X_MIN_MAX_CNC) && (XX<=X_MIN_MAX_CNC)) printf("%i\n",XX);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&XX);
							getchar();
						} while ((XX<-X_MIN_MAX_CNC) || (XX>X_MIN_MAX_CNC));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: X out of Range\n");
						return(EXIT_FAILURE);
					}
					scanf("%d",&XX);
					getchar();
					tx[pos++] = XX>>8;
					tx[pos++] = XX;
					
					printf("Z (+-32700): ");
					if ((ZZ>=-Z_MIN_MAX_CNC) && (ZZ<=Z_MIN_MAX_CNC)) printf("%i\n",ZZ);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&ZZ);
							getchar();
						} while ((ZZ<-Z_MIN_MAX_CNC) || (ZZ>Z_MIN_MAX_CNC));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: Z out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = ZZ>>8;
					tx[pos++] = ZZ;
					
					printf("Feed (2 to 499): ");
					if ((feed>=F_MIN) && (feed<=F_MAX)) printf("%i\n",feed);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&feed);
							getchar();
						} while ((feed<F_MIN) || (feed>F_MAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: F out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = feed>>8;
					tx[pos++] = feed;
					
					printf("H (0 to 999): ");
					if ((HH>=H_MIN) && (HH<=H_MAX)) printf("%i\n",HH);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&HH);
							getchar();
						} while ((HH<H_MIN) || (HH>H_MAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: H out of Range\n");
						return(EXIT_FAILURE);
					}
					tx[pos++] = HH>>8;
					tx[pos++] = HH;
					
					break;
		case 17:  	//Shutdown
					break;
		case 18:  	//Load last coordinates and tool position and init
					break;
		case 19:  	//Reset Errors
					msg_number = (lastsuccessful_msg+1)%256; //Reset Msg-No
					pos--;
					tx[pos++] = msg_number; 
					//With the next message, lost messages can be repeated, if they were important. (2D-Array needed for saving last messages)
					break;
		default:  	//SPI-Error "PID unkown"
					break;
	}
	
	printf("\n");
	
	used_length = pos-SPI_BYTE_LENGTH_PRAEAMBEL;
	
	//zero unused bytes
	for (pos; pos<(SPI_MSG_LENGTH-1); pos++) {
		tx[pos] = 0;
	}
	
	//CRC
	tx[pos] = CRC8(tx, SPI_BYTE_LENGTH_PRAEAMBEL, used_length);
	
	return EXIT_SUCCESS;
}

static int spi_transfer(int spi_fd) {
	int ret;
	uint8_t lostmessages=0, pid=0, crc_in=0;
	
	printf("tx-Buffer (HEX):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts("");
		printf("0x%.2X ", tx[ret]);
	}
	printf("\n");
	printf("tx-Buffer (DEC):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts("");
		printf("%.3i ", tx[ret]);
	}
	
	printf("\n\n");
	
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message"); //has to be replaced
	else { //output received Message
		//Byteorder: 100 lastsuccessful_msg byte2=bit7_stepper|bit6_spindle_direction|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active RPM_H&L XX ZZ FF HH T NN ERROR_Numbers CRC-8 #Machine State
		printf("Incomming Message:\n");
		printf("------------------\n");
		pid = rx[SPI_BYTE_ARDUINO_MSG_TYPE];
		printf("PID: %i\n", pid);
		lastsuccessful_msg = rx[SPI_BYTE_ARDUINO_MSG_LASTSUCCESS_MSG_NO];
		printf("Last Successful Message: %i\n", lastsuccessful_msg);
		STATE_active = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_CONTROL_ACTIVE_BIT)&1;
		printf("Control active: %i\n", STATE_active);
		STATE_init = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_INIT_BIT)&1;
		printf("init: %i\n", STATE_init);
		STATE_manual = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_MANUAL_BIT)&1;
		printf("manual: %i\n", STATE_manual);
		STATE_pause = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_PAUSE_BIT)&1;
		printf("Pause: %i\n", STATE_pause);
		STATE_inch = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_INCH_BIT)&1;
		printf("inch: %i\n", STATE_inch);
		STATE_spindle_on = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_SPINDLE_BIT)&1;
		printf("Spindle on: %i\n", STATE_spindle_on);
		STATE_spindle_direction = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_SPINDLE_DIRECTION_BIT)&1;
		printf("Spindle direction: %i\n", STATE_spindle_direction);
		STATE_stepper_on = (rx[SPI_BYTE_ARDUINO_MSG_STATE]>>STATE_STEPPER_BIT)&1;
		printf("Stepper on: %i\n", STATE_stepper_on);
		STATE_RPM = (((int)rx[SPI_BYTE_ARDUINO_RPM_H]<<8)|(rx[SPI_BYTE_ARDUINO_RPM_L]));
		printf("RPM: %i\n", STATE_RPM);
		STATE_X = (((int)rx[SPI_BYTE_ARDUINO_X_H]<<8)|(rx[SPI_BYTE_ARDUINO_X_L]));
		printf("X: %i\n", STATE_X);
		STATE_Z = (((int)rx[SPI_BYTE_ARDUINO_Z_H]<<8)|(rx[SPI_BYTE_ARDUINO_Z_L]));
		printf("Z: %i\n", STATE_Z);
		STATE_F = (((int)rx[SPI_BYTE_ARDUINO_F_H]<<8)|(rx[SPI_BYTE_ARDUINO_F_L]));
		printf("F: %i\n", STATE_F);
		STATE_H = (((int)rx[SPI_BYTE_ARDUINO_H_H]<<8)|(rx[SPI_BYTE_ARDUINO_H_L]));
		printf("H: %i\n", STATE_H);
		STATE_T = rx[SPI_BYTE_ARDUINO_T];
		printf("T: %i\n", STATE_T);
		STATE_N = (((int)rx[SPI_BYTE_ARDUINO_N_H]<<8)|(rx[SPI_BYTE_ARDUINO_N_L]));
		printf("Block-No: %i\n", STATE_N);
		ERROR_spi_error = (rx[SPI_BYTE_ARDUINO_ERROR_NO]>>ERROR_SPI_BIT)&1;
		printf("SPI-Error: %i\n", ERROR_spi_error);
		ERROR_cnc_code_error = (rx[SPI_BYTE_ARDUINO_ERROR_NO]>>ERROR_CNC_CODE_BIT)&1;
		printf("CNC-Code-Error: %i\n", ERROR_cnc_code_error);
		ERROR_spindle_error = (rx[SPI_BYTE_ARDUINO_ERROR_NO]>>ERROR_SPINDLE_BIT)&1;
		printf("Spindle-Error: %i\n", ERROR_spindle_error);
		crc_in = rx[SPI_BYTE_ARDUINO_CRC8];
		printf("CRC: %i\n\n", crc_in);
		
		if (CRC8(rx, SPI_BYTE_LENGTH_PRAEAMBEL, SPI_MSG_LENGTH-SPI_BYTE_LENGTH_PRAEAMBEL) || pid != 100) {
			printf("CRC- or PID-Check of incomming message failed!!!\n\n");
			/*
			//should be implemented in another way
			printf("Try to get Status Update!\n");
			while (spi_transfer(spi_fd, NULL, 1)) {
					usleep(500000); //0,5s
			}
			*/
		}
		else {
			//Inform about lost messages
			lostmessages = msg_number-lastsuccessful_msg-1;
			if (lostmessages<0) lostmessages += 256;
			//if (msg_number != (lastsuccessful_msg+1)%256) {
			if (lostmessages>0) {
				printf("%i messages to arduino lost or ignored by arduino after send-error!!!\n\n", lostmessages);
			}
			
			//Ouptut to Machine-State-File
			machinestatefile = fopen(MACHINE_STATE_FILE, "w");
			if (spi_fd < 0) printf("can't open Machine State file\n");
			else {
				fprintf(machinestatefile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
				fprintf(machinestatefile, "<machinestate>\n");
				fprintf(machinestatefile, "\t<state>\n");
				fprintf(machinestatefile, "\t\t<active>%i</active>\n", STATE_active);
				fprintf(machinestatefile, "\t\t<init>%i</init>\n", STATE_init);
				fprintf(machinestatefile, "\t\t<manual>%i</manual>\n", STATE_manual);
				fprintf(machinestatefile, "\t\t<pause>%i</pause>\n", STATE_pause);
				fprintf(machinestatefile, "\t\t<inch>%i</inch>\n", STATE_inch);
				fprintf(machinestatefile, "\t\t<spindle_on>%i</spindle_on>\n", STATE_spindle_on);
				fprintf(machinestatefile, "\t\t<spindle_direction>%i</spindle_direction>\n", STATE_spindle_direction);
				fprintf(machinestatefile, "\t\t<stepper_on>%i</stepper_on>\n", STATE_stepper_on);
				fprintf(machinestatefile, "\t</state>\n");
				fprintf(machinestatefile, "\t<measure>\n");
				fprintf(machinestatefile, "\t\t<rpm_measure>%i</rpm_measure>\n", STATE_RPM);
				fprintf(machinestatefile, "\t\t<x_actual>%i</x_actual>\n", STATE_X);
				fprintf(machinestatefile, "\t\t<z_actual>%i</z_actual>\n", STATE_Z);
				fprintf(machinestatefile, "\t\t<f_actual>%i</f_actual>\n", STATE_F);
				fprintf(machinestatefile, "\t\t<h_actual>%i</h_actual>\n", STATE_H);
				fprintf(machinestatefile, "\t\t<t_actual>%i</t_actual>\n", STATE_T);	
				fprintf(machinestatefile, "\t</measure>\n");
				fprintf(machinestatefile, "\t<error>\n");
				fprintf(machinestatefile, "\t\t<spi_error>%i</spi_error>\n", ERROR_spi_error);
				fprintf(machinestatefile, "\t\t<cnc_code_error>%i</cnc_code_error>\n", ERROR_cnc_code_error);
				fprintf(machinestatefile, "\t\t<spindle_error>%i</spindle_error>\n", ERROR_spindle_error);
				fprintf(machinestatefile, "\t</error>\n");
				fprintf(machinestatefile, "\t<cncblock>\n");
				fprintf(machinestatefile, "\t\t<n_actual>%i</n_actual>\n", STATE_N);
				fprintf(machinestatefile, "\t</cncblock>\n");
				fprintf(machinestatefile, "</machinestate>\n");

				printf("close(machinestatefile)\n\n");
				fclose(machinestatefile);
			}
		}
	}
	
	//output rx-Buffer
	printf("rx-Buffer (HEX):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts("");
		printf("0x%.2X ", rx[ret]);
	}
	printf("\n");
	printf("rx-Buffer (DEC):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts("");
		printf("%.3i ", rx[ret]);
	}
	
	puts("");
	printf("\n");
	
	msg_number++; //for next message
	return lostmessages;
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n"
		 "  -N --no-cs    no chip select?\n"
	     "  -R --ready    SPI_READY\n"
		 "  -m --manual   manual input of msg (no pipe-server)\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ "manual",  0, 0, 'm' },
			{ "verbose", 0, 0, 'v' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR:m:v", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		case 'm':
			start_pipe_server = 0;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{	
	parse_opts(argc, argv);
	
	//allow all rights for new created files
	umask(0);
	
	if (start_pipe_server) setup_pipe_server();

	if (setup_spi() == 0){
		while(1) { //Server-Loop
			if (start_pipe_server) {
				/* Initialize the file descriptor r_fd_set. */
				// has to be done every iteration, but why?
				FD_ZERO(&r_fd_set);
				FD_SET(r_fd, &r_fd_set);
			
				/* Initialize the timeout data structure. */
				// has to be done every iteration, because select may update the timeout argument to indicate how much time was left
				timeout.tv_sec = 1; //seconds
				timeout.tv_usec = 0; //microseconds
			
				/* In the interest of brevity, I'm using the constant FD_SETSIZE, but a more
				   efficient implementation would use the highest fd + 1 instead. In this case
				   with a single fd, you can replace FD_SETSIZE with
				   r_fd+1 thereby limiting the number of fds the system has to
				   iterate over. */
				//ret = select(FD_SETSIZE, &r_fd_set, NULL, NULL, &timeout);
				ret = select(r_fd+1, &r_fd_set, NULL, NULL, &timeout);

				if (ret == 0) //timeout
				{
					//printf("select timeout after 1s waiting for message on pipe!\n");
					if (verbose) printf("Updating Machine-State after waiting for message on pipe for 1s!\n");
					if (!spi_create_command_msg(NULL, 1)) messages_notreceived = spi_transfer(spi_fd);
				}
				else if (ret < 0) //error
				{
					printf("select returned with an error while waiting for message on pipe!\n");
				}
				else //there was activity on the file descripor
				{
					//printf("select says pipe is readable\n");
					//fscanf(r_fz,"%s\n%d", client_sid, &pid);
					
					if (read (r_fd, buffer[ringbuffer_pos], BUF) != 0) {
						/*
						//debug
						printf("buffer %p:\n", buffer[ringbuffer_pos]);
						for (i=0; i<BUF; i++){
							printf("%c 0x%x, ", buffer[ringbuffer_pos][i], buffer[ringbuffer_pos][i]);
						}
						printf("\n");
						*/
						/*
						//get SESSION-ID of calling client
						i = 0;
						while (buffer[i] != '\n') {
							client_sid[i] = buffer[i];
							i++;
						}
						
						//get SESSION-ID of calling client
						pid = ;
						*/

						/*
						//set answer_fifo_name
						strcpy (answer_fifo_name, "client_session_pipe.");
						strncat (answer_fifo_name, client_sid, i);

						//set answer to client
						answer_to_client[0] = '\0';
						if ( exclusive != atoi(client_sid)) strcpy (answer_to_client, MSG_ERROR_CLIENT);
						else {
							if (!state) strcpy (answer_to_client, MSG_SUCCESS);
							else strcpy (answer_to_client, MSG_ERROR_STATE);
						}
										
						if (strlen(answer_to_client)) {
							//open client_session_pipe send answer
							w_fd = open (answer_fifo_name, O_WRONLY);
							if (w_fd == -1) {
								perror("Backend Answer-Pipe: open(2)");
								exit (EXIT_FAILURE);
							}
							write (w_fd, answer_to_client, strlen(answer_to_client));
							close (w_fd);
						}
						*/
						
						if (ringbuffer_fill_status<=SPI_TX_RINGBUFFERSIZE) ringbuffer_fill_status++;
						
						//process message
						if (!spi_create_command_msg(buffer[ringbuffer_pos], 0)) messages_notreceived = spi_transfer(spi_fd); //status-update needed! Warning may come later, exspecially when CRC- or PID-Check of incomming msg fails.
						if (process_file) {
							usleep(200000); //0,2s
							//if(!file_parser()) spi_create_cnc_code_messages(); //create and send messages from file
							process_file = 0;
						}
						/*
						//Error-Handling not ready
						if (messages_notreceived) {
							messages_to_repeat += messages_notreceived; //not finished
							
							while(messages_to_repeat) {
								//Reset SPI-Error
								while (messages_notreceived) {	
										if (!spi_create_command_msg(NULL, 19)) messages_notreceived = spi_transfer(spi_fd);
										usleep(200000); //0,2s
										if (!spi_create_command_msg(NULL, 1)) messages_notreceived = spi_transfer(spi_fd);
										
								}
								
								//try to send lost messages again
								if (messages_notreceived>SPI_TX_RINGBUFFERSIZE) {
									//error handling needed!
									printf("%i Messages lost!\n", messages_notreceived);
								}
								else {
									//Update ringbuffer_pos
									if (messages_notreceived>ringbuffer_fill_status) ringbuffer_pos -= ringbuffer_fill_status;
									else ringbuffer_pos -= messages_notreceived;
									if (ringbuffer_pos<0) ringbuffer_pos += SPI_TX_RINGBUFFERSIZE;
									
									if (!spi_create_command_msg(buffer[ringbuffer_pos], 0))	messages_notreceived = spi_transfer(spi_fd);
								}
							}
						} 
						*/
						
						//Update ringbuffer_pos
						if (ringbuffer_pos<SPI_TX_RINGBUFFERSIZE-1) ringbuffer_pos++;
						else ringbuffer_pos = 0;
					}
				}
			}
			else if (!spi_create_command_msg(NULL, 0)) messages_notreceived = spi_transfer(spi_fd); //commandline-mode
		}
	}
	else exit(EXIT_FAILURE);
	
	return EXIT_SUCCESS;
}
