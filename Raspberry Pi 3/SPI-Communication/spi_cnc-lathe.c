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


//defines
//#######

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
#define SPI_MSG_LENGTH (19+SPI_BYTE_LENGTH_PRAEAMBEL)
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

//Input Parameter Ranges
#define CNC_CODE_NMIN 0
#define CNC_CODE_NMAX 499
#define CNC_CODE_FILE_PARSER_NMAX 9999
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
#define SPI_BYTE_ARDUINO_MSG_STATE1 (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_MSG_STATE2 (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_RPM_H (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_RPM_L (5+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_X_H (6+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_X_L (7+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_Z_H (8+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_Z_L (9+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_F_H (10+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_F_L (11+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_H_H (12+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_H_L (13+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_T (14+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_N_H (15+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_N_L (16+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_ERROR_NO (17+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_ARDUINO_CRC8 (SPI_MSG_LENGTH-1)

//Byte Postions of RASPI-Msg
#define SPI_BYTE_RASPI_MSG_TYPE (0+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_NO (1+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_N_H (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_N_L (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_OFFSET_N_H (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_OFFSET_N_L (3+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_MAX_N_H (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_MAX_N_L (5+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_GM_TYPE (4+SPI_BYTE_LENGTH_PRAEAMBEL)
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
#define SPI_BYTE_RASPI_MSG_TOOL_T (6+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_INCH (2+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_G_INCH (4+SPI_BYTE_LENGTH_PRAEAMBEL)
#define SPI_BYTE_RASPI_MSG_CRC8 (SPI_MSG_LENGTH-1)

//Bit Postions of STATE
#define STATE1_CONTROL_ACTIVE_BIT 0
#define STATE1_INIT_BIT 1
#define STATE1_MANUAL_BIT 2
#define STATE1_PAUSE_BIT 3
#define STATE1_INCH_BIT 4
#define STATE1_SPINDLE_BIT 5
#define STATE1_SPINDLE_DIRECTION_BIT 6
#define STATE1_STEPPER_BIT 7

//Bit Postions of STATE2
#define STATE2_COMMAND_TIME_BIT 0
#define STATE2_XSTEPPER_RUNNING_BIT 1
#define STATE2_ZSTEPPER_RUNNING_BIT 2
#define STATE2_TOOLCHANGER_RUNNING_BIT 3
#define STATE2_CNC_CODE_NEEDED_BIT 4

//Bit Postions of ERROR_NO (actual ERROR-Numbers Bit-coded)
#define ERROR_SPI_BIT 0
#define ERROR_CNC_CODE_BIT 1
#define ERROR_SPINDLE_BIT 2

#define MACHINE_STATE_FILE "/var/www/html/xml/machine_state.xml"
//#define MACHINE_STATE_FILE "~/machine_state.xml" //does not work
//#define MACHINE_STATE_FILE "machine_state.xml"

//File-Parser
#define FILE_CNC_CODE "/var/www/html/uploads/cnc_code.txt"
#define LINELENGTH 80

//development switches
//#define FILEPARSER_STANDALONE


//includes
//#######

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


//global vars
//###########

//Machine-State
uint8_t STATE_T=0, STATE_active=0, STATE_init=0, STATE_manual=0, STATE_pause=0, STATE_inch=0, STATE_spindle_on=0, STATE_spindle_direction=0, STATE_stepper_on=0, STATE2_command_time=0, STATE2_xstepper_running=0, STATE2_zstepper_running=0, STATE2_toolchanger_running=0, STATE2_cnc_code_needed=0, ERROR_spi_error=0, ERROR_cnc_code_error=0, ERROR_spindle_error=0;
int16_t STATE_N=0, STATE_N_Offset=0, STATE_RPM=0,  STATE_X=0, STATE_Z=0, STATE_F=0, STATE_H=0;

//spi-Master
char spi_open = 0, machinestatefile_open = 0;
FILE *machinestatefile;
int spi_fd;
uint8_t msg_number=1, lastsuccessful_msg =0;
//uint8_t tx[SPI_MSG_LENGTH] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0};
uint8_t tx[SPI_MSG_LENGTH] = {0x7F,0xFF,0x7F,0xFF, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0};
static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 122000; //max speed in Hz (at 500000 Hz the Arduino receives not all bytes for sure)
static uint16_t delay;

//pipe-Server
char start_pipe_server=1, verbose = 1, state=0, client_sid[CLIENT_SESSION_ID_ARRAY_LENGTH], exclusive[CLIENT_SESSION_ID_ARRAY_LENGTH], buffer[SPI_TX_RINGBUFFERSIZE][BUF], answer_to_client[BUF], answer_fifo_name[BUF];
int r_fd, w_fd, i, ret, ringbuffer_pos=0, messages_notreceived=0, ringbuffer_fill_status=0;
//FILE *r_fz, *w_fz;
//parameter for select
fd_set r_fd_set;
struct timeval timeout;

//File-Parser
FILE *cnc_code_file;
int cnc_code_array_length = 0, N_Offset_next=0;
char file_code_sent_to_arduino=0;
struct cnc_code_block {
	unsigned int N; //block-No.
	char GM; //G or M-Code
	unsigned char GM_NO; //G/M-Code-Number
	int XI; //X/I-Parameter
	int ZK; //Z/K-Parameter (K for M99)
	int FTLK; //F/T/L/K-Parameter (K for G33 and G78)
	int HS; //H/S-Parameter
};
struct cnc_code_block *cnc_code_array = NULL;
struct stat cnc_code_file_attributes;
time_t processed_file_m_time=0;


//functions
//#########
static void pabort(const char *);
void signal_callback_handler(int);
static void print_usage(const char *);
static void parse_opts(int, char **);
void setup_pipe_server();
int setup_spi();
uint8_t _crc8_ccitt_update (uint8_t, uint8_t);
uint8_t CRC8 (uint8_t *, uint8_t, uint8_t);
static int spi_create_command_msg(const char *, char);
static int spi_transfer(int);
int test_value_range(int, char, int, int, int);
int get_next_cnc_code_parameter(int, int *, char *, int *, char, int *, char, int, int);
int file_parser_abort();
int file_parser();
int get_offset(int);
static int spi_create_cnc_code_messages(int);


static void pabort(const char *s) {
	perror(s);
	abort();
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void signal_callback_handler(int signum) {
	printf("\nCaught signal %d\n",signum);
	// Cleanup and close up stuff here
	
	if (spi_open) {
		printf("close(spi_fd)\n");
		close(spi_fd);
	}
	if (start_pipe_server) {
		printf("close(r_fd)\n");
		close(r_fd);
	}
	if (machinestatefile_open) {
		printf("fclose(machinestatefile)\n");
		fclose(machinestatefile);
	}
		
	// Terminate program
	exit(signum);
}

static void print_usage(const char *prog) {
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

static void parse_opts(int argc, char *argv[]) {
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


//Pipe-Server
//###########

void setup_pipe_server() {
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


//SPI-Master
//##########

int setup_spi() {
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
	int ret = 0;

	spi_fd = open(device, O_RDWR);
	if (spi_fd < 0)
		pabort("can't open SPI-device");
	
	//set File-Handler-opened-flag for signal_callback_handler
	spi_open = 1;

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
  //ATTENTION: if all bytes are zero, CRC8-Check is always correct!!! High possibility of incorrect detected message.
  uint8_t bytecount, data, crc_8=0;
  
  for (bytecount=message_offset;bytecount<(used_message_bytes+message_offset);bytecount++) {
    data = buf[bytecount];
    crc_8 = _crc8_ccitt_update (crc_8,data);
  }
  
  return crc_8;
}

static int spi_create_command_msg(const char *pipe_msg_buffer, char msg_type) {
	int n, block=-1, StartblockMax=CNC_CODE_NMAX, FileParserOverride=-1, block_n_max=-1, block_n_offset=-1, rpm=-1, spindle_direction=-1, negativ_direction=-1, XX=32767, ZZ=32767, feed=-1, tool=0, inch=-1, gmcode=-1, HH=-1, code_type=0;
	uint8_t used_length=0, pos=SPI_BYTE_LENGTH_PRAEAMBEL;
	
	const char *pipe_msg_buffer_temp = pipe_msg_buffer; //hotfix
	
	printf("Creating new SPI-Message\n");
	printf("########################\n");
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
				return EXIT_FAILURE;
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
		return EXIT_FAILURE;
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
						n = sscanf(pipe_msg_buffer,"%s %d %d %d", client_sid, &msg_type, &block, &FileParserOverride);
						if (n != 4) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return EXIT_FAILURE;
						}
					}
					
					printf("FileParserOverride (0 or 1): "); //is not send to arduino, it is a backend-switch
					if ((FileParserOverride>=0) && (FileParserOverride<=1)) printf("%i\n",FileParserOverride);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&FileParserOverride);
							getchar();
						} while ((FileParserOverride<0) || (FileParserOverride>1));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: FileParserOverride out of Range\n");
						return EXIT_FAILURE;
					}
					
					if (!FileParserOverride) StartblockMax = CNC_CODE_FILE_PARSER_NMAX;
					
					printf("Block-No (0 to %i): ", StartblockMax);
					if ((block>=CNC_CODE_NMIN) && (block<=StartblockMax)) printf("%i\n",block);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&block);
							getchar();
						} while ((block<CNC_CODE_NMIN) || (block>StartblockMax));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: N out of Range\n");
						return EXIT_FAILURE;
					}
					
					if (!FileParserOverride) {
						if (stat(FILE_CNC_CODE, &cnc_code_file_attributes)) { //get last modification time of CNC-Code_File
							perror("Backend Command-Interpreter: Error reading last modification time of CNC-Code-File");
							return EXIT_FAILURE;
						}
						if (cnc_code_file_attributes.st_mtime != processed_file_m_time) { //check if file was already processed
							if (file_parser()) { //parse new file
								perror("Backend Command-Interpreter: Could not process CNC-Code-File");
								return EXIT_FAILURE;
							}
							if (!get_offset(block)) block = N_Offset_next; //Search for flattened Startblock-address (saved in global N_Offset_next)
							else {
								perror("Backend Command-Interpreter: Startblock not in CNC-Code-File");
								return EXIT_FAILURE;
							}
							spi_create_cnc_code_messages(block); //create and send messages from file
							return EXIT_SUCCESS;
						}
						else {
							if (!get_offset(block)) block = N_Offset_next; //Search for flattened Startblock-address (saved in global N_Offset_next)
							else {
								perror("Backend Command-Interpreter: Startblock not in CNC-Code-File");
								return EXIT_FAILURE;
							}
							if (block < STATE_N_Offset || block > STATE_N_Offset+CNC_CODE_NMAX || !file_code_sent_to_arduino) { //needed Code not already uploaded to arduino?
								spi_create_cnc_code_messages(block); //create and send messages from file
								return EXIT_SUCCESS;
							}
						}
					}
					//if the needed Code was already uploaded to the arduino, just send the flattened Startblock-address
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
							return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
							return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
							return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
						return EXIT_FAILURE;
					}
					tx[pos++] = tool;
					break;
		case 12:   	//Origin-X-Offset
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d", client_sid, &msg_type, &XX);
						if (n != 3) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
							return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
							return EXIT_FAILURE;
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
						return EXIT_FAILURE;
					}
					tx[pos++] = inch;
					break;
		case 15:  	//New CNC-Programm wit N Blocks in metric or inch (maybe not used, instead File is uploaded by file-parser)
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %d %d", client_sid, &msg_type, &block_n_offset, &block_n_max, &inch);
						if (n != 4) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return EXIT_FAILURE;
						}
					}
					
					printf("N-Offset (0 to 9999): ");
					if ((block_n_offset>=CNC_CODE_NMIN) && (block_n_offset<=CNC_CODE_FILE_PARSER_NMAX)) printf("%i\n",block_n_offset);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&block_n_offset);
							getchar();
						} while ((block_n_offset<CNC_CODE_NMIN) || (block_n_offset>CNC_CODE_FILE_PARSER_NMAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: N out of Range\n");
						return EXIT_FAILURE;
					}
					tx[pos++] = block_n_offset>>8;
					tx[pos++] = block_n_offset;
					
					printf("N-Max (0 to 9999): ");
					if ((block_n_max>=CNC_CODE_NMIN) && (block_n_max<=CNC_CODE_FILE_PARSER_NMAX)) printf("%i\n",block_n_max);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&block_n_max);
							getchar();
						} while ((block_n_max<CNC_CODE_NMIN) || (block_n_max>CNC_CODE_FILE_PARSER_NMAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: N out of Range\n");
						return EXIT_FAILURE;
					}
					tx[pos++] = block_n_max>>8;
					tx[pos++] = block_n_max;
					
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
						return EXIT_FAILURE;
					}
					tx[pos++] = inch;
					file_code_sent_to_arduino=0;
					break;
		case 16:  	//CNC-Code-Block (maybe not used, instead File is uploaded by file-parser)
					if (pipe_msg_buffer != NULL) {
						n = sscanf(pipe_msg_buffer,"%s %d %c %d %d %d %d %d", client_sid, &msg_type, &block, &code_type, &gmcode, &XX, &ZZ, &feed, &HH);
						if (n != 9) {
							if (errno != 0) perror("Backend Command-Interpreter: scanf");
							else fprintf(stderr, "Backend Command-Interpreter: Parameter not matching\n");
							return EXIT_FAILURE;
						}
					}
					
					printf("Block-No (0 to 499): ");
					if ((block>=CNC_CODE_NMIN) && (block<=CNC_CODE_NMAX)) printf("%i\n",block);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&block);
							getchar();
						} while ((block<CNC_CODE_NMIN) || (block>CNC_CODE_NMAX));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: N out of Range\n");
						return EXIT_FAILURE;
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
						return EXIT_FAILURE;
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
							return EXIT_FAILURE;
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
							return EXIT_FAILURE;
						}
					}
					tx[pos++] = gmcode;
					
					printf("X or I (+-5999): ");
					if ((XX>=-X_MIN_MAX_CNC) && (XX<=X_MIN_MAX_CNC)) printf("%i\n",XX);
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&XX);
							getchar();
						} while ((XX<-X_MIN_MAX_CNC) || (XX>X_MIN_MAX_CNC));
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: X or I out of Range\n");
						return EXIT_FAILURE;
					}
					scanf("%d",&XX);
					getchar();
					tx[pos++] = XX>>8;
					tx[pos++] = XX;
					
					printf("Z (+-32700) or K (+-5999): ");
					if ((ZZ>=-Z_MIN_MAX_CNC) && (ZZ<=Z_MIN_MAX_CNC)) printf("%i\n",ZZ); //not right!!! Many cases!!!
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&ZZ);
							getchar();
						} while ((ZZ<-Z_MIN_MAX_CNC) || (ZZ>Z_MIN_MAX_CNC)); //not right!!! Many cases!!!
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: Z or K out of Range\n");
						return EXIT_FAILURE;
					}
					tx[pos++] = ZZ>>8;
					tx[pos++] = ZZ;
					
					printf("F (2 to 499), T (0 to 6), L (+-9999), K (0 to 499 (Thread Pitch) or 5999: ");
					if ((feed>=-CNC_CODE_FILE_PARSER_NMAX) && (feed<=CNC_CODE_FILE_PARSER_NMAX)) printf("%i\n",feed); //not right!!! Many cases!!!
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&feed);
							getchar();
						} while ((feed<-CNC_CODE_FILE_PARSER_NMAX) || (feed>CNC_CODE_FILE_PARSER_NMAX)); //not right!!! Many cases!!!
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: F,T,L or K out of Range\n");
						return EXIT_FAILURE;
					}
					tx[pos++] = feed>>8;
					tx[pos++] = feed;
					
					printf("H (0 to 999) or S (460 to 3220: ");
					if ((HH>=H_MIN) && (HH<=REVOLUTIONS_MAX)) printf("%i\n",HH);  //not right!!! Many cases!!!
					else if (pipe_msg_buffer == NULL) {
						do {
							scanf("%d",&HH);
							getchar();
						} while ((HH<H_MIN) || (HH>REVOLUTIONS_MAX)); //not right!!! Many cases!!!
					}
					else {
						fprintf(stderr, "Backend Command-Interpreter: H or S out of Range\n");
						return EXIT_FAILURE;
					}
					tx[pos++] = HH>>8;
					tx[pos++] = HH;
					file_code_sent_to_arduino=0;
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
	
	printf("SPI-Transfer\n");
	printf("------------\n");
	printf("Message-No: %i\n\n", msg_number);
	
	printf("tx-Buffer (HEX):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts(""); //newline
		printf("0x%.2X ", tx[ret]);
	}
	printf("\n");
	printf("tx-Buffer (DEC):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts(""); //newline
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
		STATE_active = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_CONTROL_ACTIVE_BIT)&1;
		printf("Control active: %i\n", STATE_active);
		STATE_init = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_INIT_BIT)&1;
		printf("init: %i\n", STATE_init);
		STATE_manual = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_MANUAL_BIT)&1;
		printf("manual: %i\n", STATE_manual);
		STATE_pause = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_PAUSE_BIT)&1;
		printf("Pause: %i\n", STATE_pause);
		STATE_inch = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_INCH_BIT)&1;
		printf("inch: %i\n", STATE_inch);
		STATE_spindle_on = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_SPINDLE_BIT)&1;
		printf("Spindle on: %i\n", STATE_spindle_on);
		STATE_spindle_direction = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_SPINDLE_DIRECTION_BIT)&1;
		printf("Spindle direction: %i\n", STATE_spindle_direction);
		STATE_stepper_on = (rx[SPI_BYTE_ARDUINO_MSG_STATE1]>>STATE1_STEPPER_BIT)&1;
		printf("Stepper on: %i\n", STATE_stepper_on);
		STATE2_command_time = (rx[SPI_BYTE_ARDUINO_MSG_STATE2]>>STATE2_COMMAND_TIME_BIT)&1;
		printf("Command time: %i\n", STATE2_command_time);
		STATE2_xstepper_running = (rx[SPI_BYTE_ARDUINO_MSG_STATE2]>>STATE2_XSTEPPER_RUNNING_BIT)&1;
		printf("X-Stepper running: %i\n", STATE2_xstepper_running);
		STATE2_zstepper_running = (rx[SPI_BYTE_ARDUINO_MSG_STATE2]>>STATE2_ZSTEPPER_RUNNING_BIT)&1;
		printf("Z-Stepper running: %i\n", STATE2_zstepper_running);
		STATE2_toolchanger_running = (rx[SPI_BYTE_ARDUINO_MSG_STATE2]>>STATE2_TOOLCHANGER_RUNNING_BIT)&1;
		printf("Toolchanger running: %i\n", STATE2_toolchanger_running);
		STATE2_cnc_code_needed = (rx[SPI_BYTE_ARDUINO_MSG_STATE2]>>STATE2_CNC_CODE_NEEDED_BIT)&1;
		printf("CNC-Code needed: %i\n", STATE2_cnc_code_needed);
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
			
			//get last modification time of CNC-Code_File
			if (stat(FILE_CNC_CODE, &cnc_code_file_attributes)) perror("Error reading last modification time of CNC-Code_File");
			
			//Ouptut to Machine-State-File
			machinestatefile = fopen(MACHINE_STATE_FILE, "w");
			if (spi_fd < 0) printf("can't open Machine State file\n");
			else {
				machinestatefile_open = 1;
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
				fprintf(machinestatefile, "\t\t<command_time>%i</command_time>\n", STATE2_command_time);
				fprintf(machinestatefile, "\t\t<xstepper_running>%i</xstepper_running>\n", STATE2_xstepper_running);
				fprintf(machinestatefile, "\t\t<zstepper_running>%i</zstepper_running>\n", STATE2_zstepper_running);
				fprintf(machinestatefile, "\t\t<toolchanger_running>%i</toolchanger_running>\n", STATE2_toolchanger_running);
				fprintf(machinestatefile, "\t\t<cnc_code_needed>%i</cnc_code_needed>\n", STATE2_cnc_code_needed);
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
				if(file_code_sent_to_arduino) fprintf(machinestatefile, "\t\t<n_actual>%i</n_actual>\n", cnc_code_array[STATE_N].N);
				else fprintf(machinestatefile, "\t\t<n_actual>%i</n_actual>\n", STATE_N);
				fprintf(machinestatefile, "\t</cncblock>\n");
				fprintf(machinestatefile, "\t<cncfile>\n");
				fprintf(machinestatefile, "\t\t<mtime>%ld</mtime>\n", cnc_code_file_attributes.st_mtime);
				fprintf(machinestatefile, "\t</cncfile>\n");
				fprintf(machinestatefile, "</machinestate>\n");

				printf("close(machinestatefile)\n\n");
				fclose(machinestatefile);
				machinestatefile_open = 0;
			}
		}
	}
	
	//output rx-Buffer
	printf("rx-Buffer (HEX):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts(""); //newline
		printf("0x%.2X ", rx[ret]);
	}
	printf("\n");
	printf("rx-Buffer (DEC):");
	for (ret = 0; ret < ARRAY_SIZE(tx); ret++) {
		if (!(ret % 4))
			puts(""); //newline
		printf("%.3i ", rx[ret]);
	}
	
	printf("\n\n");
	
	msg_number++; //for next message
	return lostmessages;
}


//File-Parser
//###########

int test_value_range(int line_number, char name, int value, int min, int max) {
	#ifdef FILEPARSER_STANDALONE
		printf("Backend File-Parser debug: Line %i: Parameter %c: %i\n", line_number, name, value);
	#endif
	//test if range of value matches
	if (value < min || value > max) {
		fprintf(stderr, "Backend File-Parser: Line %i: %c out of Range\n", line_number, name);
		return 0;
	}
	return 1;
}
	
int get_next_cnc_code_parameter(int line_number, int *InputParameterNumber, char *InputParameterName, int *InputParameter, char name, int *OutputValue, char optional, int min, int max) {
	int success = 1;
	//check Name of Parameter
	#ifdef FILEPARSER_STANDALONE
		printf("Backend File-Parser debug: Line %i: InputParameterName %c, Searching for %c, InputParameter %i\n", line_number, *InputParameterName, name, *InputParameter);
	#endif
	if (*InputParameterName == name) {
		//test range of value matches
		if (success = test_value_range(line_number, name, *InputParameter, min, max)) {
			*OutputValue=*InputParameter;
			(*InputParameterNumber)++;
		}
		//else return 0;
	}
	else {
		*OutputValue=0; //set Default Value
		if (!optional) {
			fprintf(stderr, "Backend File-Parser: Line %i: no %c-Parameter or incorrect format\n", line_number, name);
			//return 0;
		} //else return 1;
		success = 0;
	}
	return success;
	//return 2; //Parameter found
}

int file_parser_abort() {
	fclose(cnc_code_file);
	return EXIT_FAILURE;
}

int file_parser() {
	printf("Backend File-Parser: File-Parser\n");
	
	struct cnc_code_block_raw {
		unsigned int N; //block-No.
		char GM; //G or M-Code
		unsigned char GM_NO; //G/M-Code-Number
		char c1; //Name of 1. Parameter
		int p1; //1. Parameter
		char c2; //Name of 2. Parameter
		int p2; //2. Parameter
		char c3; //Name of 3. Parameter
		int p3; //3. Parameter
		char c4; //Name of 4. Parameter
		int p4; //4. Parameter
	} cnc_code_block_raw;

	cnc_code_file = fopen(FILE_CNC_CODE, "r");
	
	char c = 0, success=0;
	char line[LINELENGTH] = {};
	int n = 0, i = 0, j = 0, StartSignLine = 0, StopSignLine = 0, ret = 0;
	fpos_t startpos;
	char *InputParameterName[4];
	int *InputParameter[4];
	
	cnc_code_array_length =0;
	
	
	//find start- and stop-signs '%'
	while (c != EOF){
		StopSignLine++; //stopsign not needed
		fgets(line, LINELENGTH, cnc_code_file);
		n = sscanf(line,"%c", &c);
		if (n < 1) {
			if (errno != 0) {
				perror("Backend File-Parser: scanf");
				return file_parser_abort();
			}
		}
		else if(c == '%') {
			if (!StartSignLine) {
				StartSignLine = StopSignLine;
				fgetpos(cnc_code_file,&startpos);
			}
			else break; //StopSignLine
		}
	}
	//end if no cnc-code found between start- and stop-signs
	if (!StartSignLine || StopSignLine-StartSignLine<1) {
		return file_parser_abort();
	}
	
	//rewind file-pointer to StartSignLine+1
	fsetpos(cnc_code_file,&startpos);
	int line_number = StartSignLine+1;
	
	//free old cnc_code_array
	if(cnc_code_array != NULL) free(cnc_code_array);
	
	//reserve memory for cnc_code_array (should be global)
	cnc_code_array_length = StopSignLine-StartSignLine-1;
	cnc_code_array = (struct cnc_code_block*) calloc(cnc_code_array_length, sizeof(struct cnc_code_block));  //zero it with calloc
	if(cnc_code_array == NULL) {
		perror("Backend File-Parser: no memory");
		return file_parser_abort();
	}
	//static struct cnc_code_block cnc_code_array[cnc_code_array_length] = {0}; //zero it
	
	printf("Backend File-Parser: StartSignLine: %i\n", StartSignLine);
	printf("Backend File-Parser: StopSignLine: %i\n", StopSignLine);
	#ifdef FILEPARSER_STANDALONE
		printf("Backend File-Parser debug: cnc_code_array_length: %i\n", cnc_code_array_length);
	#endif
	
	//read cnc-code
	for (i=0; i<cnc_code_array_length; i++) {
		fgets(line, LINELENGTH, cnc_code_file);
		n = sscanf(line,"N%d %c%d %c%d %c%d %c%d %c%d", &(cnc_code_block_raw.N), &(cnc_code_block_raw.GM), &(cnc_code_block_raw.GM_NO), &cnc_code_block_raw.c1, &cnc_code_block_raw.p1, &cnc_code_block_raw.c2, &cnc_code_block_raw.p2, &cnc_code_block_raw.c3, &cnc_code_block_raw.p3, &cnc_code_block_raw.c4, &cnc_code_block_raw.p4);
		if (n < 3) {
			if (errno != 0) perror("Backend File-Parser: scanf");
			else fprintf(stderr, "Backend File-Parser: Line %i: Parameter not matching\n");
			return file_parser_abort();
		}
		
		#ifdef FILEPARSER_STANDALONE
			printf("Backend File-Parser debug: Line %i: N%04i %c%i %c%i %c%i %c%i %c%i\n", line_number, cnc_code_block_raw.N, cnc_code_block_raw.GM, cnc_code_block_raw.GM_NO, cnc_code_block_raw.c1, cnc_code_block_raw.p1, cnc_code_block_raw.c2, cnc_code_block_raw.p2, cnc_code_block_raw.c3, cnc_code_block_raw.p3, cnc_code_block_raw.c4, cnc_code_block_raw.p4);
		#endif
		
		j=0;
		InputParameterName[0] = &cnc_code_block_raw.c1;
		InputParameter[0] = &cnc_code_block_raw.p1;
		InputParameterName[1] = &cnc_code_block_raw.c2;
		InputParameter[1] = &cnc_code_block_raw.p2;
		InputParameterName[2] = &cnc_code_block_raw.c3;
		InputParameter[2] = &cnc_code_block_raw.p3;
		InputParameterName[3] = &cnc_code_block_raw.c4;
		InputParameter[3] = &cnc_code_block_raw.p4;
		
		//process Code-Line
		if (cnc_code_block_raw.GM == 'G') {
			switch (cnc_code_block_raw.GM_NO){
				case 0:
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						break;
				case 1:
				case 2:
				case 3:
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						break;
				case 4:
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 0, 0, X_DWELL_MIN_MAX_CNC); //Ranges correct?
						if (!ret) return file_parser_abort();
						break;
				case 20:
				case 21:
						break;
				case 22:
						//M30[line] = N;
						break;
				case 24:
						break;
				case 25: //G25(L); //Subroutine Call-Up (returns to next block)
						/*
						if (get_next_cnc_code_parameter($Parameter, $line, $N, "L", $L, 0, CNC_CODE_NMIN, CNC_CODE_NMAX)) $G25[$N] = $L;
						else $success = 0;
						*/
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'L', &cnc_code_array[i].FTLK, 0, CNC_CODE_NMIN, CNC_CODE_NMAX);
						if (!ret) return file_parser_abort();
						break;
				case 26: //G26(X,Z,T);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'T', &cnc_code_array[i].FTLK, 0, T_MIN, T_MAX);
						if (!ret) return file_parser_abort();
						break;
				case 27: //G27(L); //Jump
						/*
						if (get_next_cnc_code_parameter($Parameter, $line, $N, "L", $L, 0, CNC_CODE_NMIN, CNC_CODE_NMAX)) $jumps[$N] = $L;
						else $success = 0;
						*/
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'L', &cnc_code_array[i].FTLK, 0, CNC_CODE_NMIN, CNC_CODE_NMAX);
						if (!ret) return file_parser_abort();
						break;
				case 33: //G33(Z,K);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'K', &cnc_code_array[i].ZK, 0, IK_MIN, K_MAX); //Ranges correct?
						if (!ret) return file_parser_abort();
						break;
				case 64:
						break;
				case 73: //G73(Z,F);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						break;
				case 78: //G78(X,Z,K,H);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'K', &cnc_code_array[i].FTLK, 0, IK_MIN, K_MAX); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'H', &cnc_code_array[i].HS, 0, H_MIN, H_MAX); //Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 81: //G81(Z,F);
				case 82: //G82(Z,F);
				case 83: //G83(Z,F);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 84: //G84(X,Z,F,H);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'H', &cnc_code_array[i].HS, 0, H_MIN, H_MAX); //optional?, Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 85: //G85(Z,F);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 86: //G86(X,Z,F,H);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'H', &cnc_code_array[i].HS, 0, H_G86_MIN, H_MAX); //optional?, Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 88: //G88(X,Z,F,H);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'H', &cnc_code_array[i].HS, 0, H_MIN, H_MAX); //optional?, Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 89: //G89(Z,F);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) return file_parser_abort();
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 90:
				case 91:
						break;
				case 92: //G92(X,Z);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						break;
				case 94:
				case 95:
				case 96:
						break;
				case 97: //G97(S);
				case 196: //G196(S);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'S', &cnc_code_array[i].HS, 1, REVOLUTIONS_MIN, REVOLUTIONS_MAX); //optional?
						if (!ret) return file_parser_abort();
						break;
				default:
						fprintf(stderr, "Backend File-Parser: Line %i: G-Code is not supported\n", line_number);
						return file_parser_abort();
			}
		}
		else if (cnc_code_block_raw.GM == 'M') {
			switch (cnc_code_block_raw.GM_NO){
				case 0:
				case 3:
				case 4:
				case 5:
						break;
				case 6: //M06(X,Z,T);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'T', &cnc_code_array[i].FTLK, 0, T_MIN, T_MAX);
						if (!ret) return file_parser_abort();
						break;
				case 17://Return from Subroutine
						//M17[line] = N;
						break;
				case 30://End of Programm
						//$M30="$line"; //last End of Programm for check of jump instructions
						//M30[line] = N;
						break;
				case 98: //M98(X,Z);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						ret |= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "Backend File-Parser: Line %i: No X- or Z-Parameter or incorrect format. At least one of them is required.\n", line_number);
							return file_parser_abort();
						}
						break;
				case 99: //M99(I,K);
						ret = get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'X', &cnc_code_array[i].XI, 0, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						ret &= get_next_cnc_code_parameter(line_number, &j, InputParameterName[j], InputParameter[j], 'Z', &cnc_code_array[i].ZK, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) return file_parser_abort();
						break;
				default:
						fprintf(stderr, "Backend File-Parser: Line %i: M-Code is not supported\n", line_number);
						return file_parser_abort();
			}
		}
		else {
			fprintf(stderr, "Backend File-Parser: Line %i: Parameter not matching\n", line_number);
			return file_parser_abort();
		}
		if (cnc_code_block_raw.N >= CNC_CODE_NMIN && cnc_code_block_raw.N <= CNC_CODE_NMAX) {
			cnc_code_array[i].N = cnc_code_block_raw.N;
		}
		else {
			fprintf(stderr, "Backend File-Parser: Line %i: N out of Range\n", line_number);
			return file_parser_abort();
		}	
		cnc_code_array[i].GM = cnc_code_block_raw.GM;
		cnc_code_array[i].GM_NO = cnc_code_block_raw.GM_NO;
		line_number++;
	}
	
	//change L-Parameter of Jumps and Subroutine-Call-Ups (needed to flatten block-numbers for arduino to array-index, to save memory)
	for (i=0; i<cnc_code_array_length; i++) {
		if (cnc_code_array[i].GM_NO == 25 || cnc_code_array[i].GM_NO == 27) { //only G-Codes with these No. exist
			for (j=0; j<cnc_code_array_length; j++) {
				if (cnc_code_array[j].N == cnc_code_array[i].FTLK) {
					cnc_code_array[i].FTLK = j;
					success=1;
					break;
				}
			}
			if (success) success=0;
			else {
				fprintf(stderr, "Backend File-Parser: N%04i: L%i target block does not exist\n", cnc_code_array[i].N, cnc_code_array[i].FTLK);
				return file_parser_abort();
			}
		}
	}
	
	fclose(cnc_code_file);
	
	if (stat(FILE_CNC_CODE, &cnc_code_file_attributes)) { //get last modification time of CNC-Code_File
		perror("Backend Command-Interpreter: Error reading last modification time of CNC-Code-File");
		return EXIT_FAILURE;
	}
	else processed_file_m_time = cnc_code_file_attributes.st_mtime; //save last modification time for processed file
	
	return EXIT_SUCCESS;
}

int get_offset(int block) {
	for (i=0; i<cnc_code_array_length; i++) {
		if (cnc_code_array[i].N == block) {
			N_Offset_next = i;
			return EXIT_SUCCESS;
		}
	}
	return EXIT_FAILURE;
}

static int spi_create_cnc_code_messages(int N_Offset) {
	char msg_type;
	unsigned int i, pos, used_length;
	
	//send Msg 15 (New Programm Code with N_Offset and N_MAX)
	msg_type = 15;
	pos=SPI_BYTE_LENGTH_PRAEAMBEL;
	tx[pos++] = msg_type;
	tx[pos++] = msg_number;
	tx[pos++] = N_Offset>>8;
	tx[pos++] = N_Offset;
	tx[pos++] = cnc_code_array_length>>8;
	tx[pos++] = cnc_code_array_length;
	tx[pos++] = 0; //metric
	
	used_length = pos-SPI_BYTE_LENGTH_PRAEAMBEL;
		
	//zero unused bytes (not needed in loop)
	for (pos; pos<(SPI_MSG_LENGTH-1); pos++) {
		tx[pos] = 0;
	}
	
	//CRC
	tx[pos] = CRC8(tx, SPI_BYTE_LENGTH_PRAEAMBEL, used_length);
	
	//send msg
	printf("File-Parser-Msg:\n");
	printf("################\n");
	printf("Message-Type: ");
	printf("%i\n\n",msg_type);
	messages_notreceived = spi_transfer(spi_fd);
	
	//Error-Handling needed!!!
	
	//send Msg 16 (New Block) for each needed block
	#ifdef FILEPARSER_STANDALONE
		printf("Backend File-Parser: Create tx[SPI_BYTE_LENGTH_PRAEAMBEL-1] - tx[SPI_MSG_LENGTH-1] for Msg:\n");
		printf("Backend File-Parser: MT  MNO  N_H  N_L   GM NO  XI_H XI_L  ZK_H ZK_L  FLTK_H,_L  HS_H HS_L\n");
	#endif
	msg_type = 16;
	for (i=N_Offset; i<=CNC_CODE_NMAX && i<cnc_code_array_length; i++) {
		pos=SPI_BYTE_LENGTH_PRAEAMBEL;
		tx[pos++] = msg_type;
		tx[pos++] = msg_number;
		tx[pos++] = i>>8; //flattened block-number
		tx[pos++] = i; //flattened block-number
		tx[pos++] = cnc_code_array[i].GM;
		tx[pos++] = cnc_code_array[i].GM_NO;
		tx[pos++] = cnc_code_array[i].XI >> 8;
		tx[pos++] = cnc_code_array[i].XI;
		tx[pos++] = cnc_code_array[i].ZK >> 8;
		tx[pos++] = cnc_code_array[i].ZK;
		tx[pos++] = cnc_code_array[i].FTLK >> 8;
		tx[pos++] = cnc_code_array[i].FTLK;
		tx[pos++] = cnc_code_array[i].HS >> 8;
		tx[pos++] = cnc_code_array[i].HS;
		
		used_length = pos-SPI_BYTE_LENGTH_PRAEAMBEL;
		
		//zero unused bytes (not needed in loop)
		for (pos; pos<(SPI_MSG_LENGTH-1); pos++) {
			tx[pos] = 0;
		}
		
		#ifndef FILEPARSER_STANDALONE
		//CRC
		tx[pos] = CRC8(tx, SPI_BYTE_LENGTH_PRAEAMBEL, used_length);
		
		//send msg
		printf("File-Parser-Msg:\n");
		printf("################\n");
		printf("Message-Type: ");
		printf("%i\n\n",msg_type);
		messages_notreceived = spi_transfer(spi_fd);
		
		//Error-Handling
		if(messages_notreceived > i) return EXIT_FAILURE;
		else {
			i -= messages_notreceived;
			//Reset SPI-Error
			while (messages_notreceived) {	
					if (!spi_create_command_msg(NULL, 19)) messages_notreceived = spi_transfer(spi_fd);
					usleep(200000); //0,2s
					if (!spi_create_command_msg(NULL, 1)) messages_notreceived = spi_transfer(spi_fd);
			}
		}
		#else
			tx[pos] = 0; //CRC
			
			//Output
			printf("Backend File-Parser: ");
			pos=SPI_BYTE_LENGTH_PRAEAMBEL;
			printf("%i  ", tx[pos++]); //msg_type
			printf("%03i  ", tx[pos++]); //msg_number;
			printf("0x%02x ", tx[pos++]); //(i+1)>>8;
			printf("0x%02x  ", tx[pos++]); //(i+1);
			printf("%c ", tx[pos++]); //cnc_code_array[i].GM;
			printf("%03i  ", tx[pos++]); //cnc_code_array[i].GM_NO;
			printf("0x%02x ", tx[pos++]); //cnc_code_array[i].XI >> 8;
			printf("0x%02x  ", tx[pos++]); //cnc_code_array[i].XI;
			printf("0x%02x ", tx[pos++]); //cnc_code_array[i].ZK >> 8;
			printf("0x%02x  ", tx[pos++]); //cnc_code_array[i].ZK;
			printf("0x%02x ", tx[pos++]); //cnc_code_array[i].FTLK >> 8;
			printf("0x%02x  ", tx[pos++]); //cnc_code_array[i].FTLK;
			printf("0x%02x ", tx[pos++]); //cnc_code_array[i].HS >> 8;
			printf("0x%02x \n", tx[pos++]); //cnc_code_array[i].HS;
		#endif
		usleep(200000); //0,2s
	}
	//send Msg 2 (Programm-Start with Startblock=0)
	msg_type = 2;
	pos=SPI_BYTE_LENGTH_PRAEAMBEL;
	tx[pos++] = msg_type;
	tx[pos++] = msg_number;
	tx[pos++] = 0; //Start-Block
	tx[pos++] = 0; //Start-Block
	
	used_length = pos-SPI_BYTE_LENGTH_PRAEAMBEL;
		
	//zero unused bytes (not needed in loop)
	for (pos; pos<(SPI_MSG_LENGTH-1); pos++) {
		tx[pos] = 0;
	}
	
	//CRC
	tx[pos] = CRC8(tx, SPI_BYTE_LENGTH_PRAEAMBEL, used_length);
	
	//send msg
	printf("File-Parser-Msg:\n");
	printf("################\n");
	printf("Message-Type: ");
	printf("%i\n\n",msg_type);
	//messages_notreceived = spi_transfer(spi_fd); //is done in main
	
	//Error-Handling needed!!!
	
	file_code_sent_to_arduino=1;
	return EXIT_SUCCESS;
}


//main
//####

int main(int argc, char *argv[]) {	
	parse_opts(argc, argv);
	
	// Register signal and signal handler
	signal(SIGINT, signal_callback_handler);
	
	//allow all rights for new created files
	umask(0);
	
	if (!stat(FILE_CNC_CODE, &cnc_code_file_attributes)) {
		printf("last modification time of CNC-Code_File: %ld\n", cnc_code_file_attributes.st_mtime); //time_t st_mtime fits in long int
	}
	else perror("Error reading last modification time of CNC-Code_File");
	
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
						
						
						//send CNC-Code to Arduino if required
						if (STATE2_cnc_code_needed && file_code_sent_to_arduino) {
							N_Offset_next = STATE_N_Offset + STATE_N;
							spi_create_cnc_code_messages(N_Offset_next); //create and send messages from file
						}
						
						//process pipe-message
						if (ringbuffer_fill_status<=SPI_TX_RINGBUFFERSIZE) ringbuffer_fill_status++;
						if (!spi_create_command_msg(buffer[ringbuffer_pos], 0)) messages_notreceived = spi_transfer(spi_fd); //status-update needed! Warning may come later, exspecially when CRC- or PID-Check of incomming msg fails.
						
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