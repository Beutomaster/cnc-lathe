/* SPI Communication Tool for an modified Emco Compact 5 CNC-Lathe
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
 */
 
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <signal.h>

 //defines
#define SPI_MSG_LENGTH 17
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

//#define MACHINE_STATE_FILE "/var/www/html/xml/machine_state.xml" //does not work
//#define MACHINE_STATE_FILE "~/machine_state.xml" //does not work
#define MACHINE_STATE_FILE "machine_state.xml"

FILE *machinestatefile;
int fd;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void
signal_callback_handler(int signum)
{
	printf("\nCaught signal %d\n",signum);
	// Cleanup and close up stuff here
	printf("close(fd)\n");
	close(fd);
		
	// Terminate program
	exit(signum);
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

uint8_t CRC8 (uint8_t * buf, uint8_t len) {
  uint8_t crc_8 = 0; //stub
  return crc_8;
}

static void transfer(int fd)
{
	int ret, block=-1, rpm=-1, msg_type=-1, spindle_direction=-1, negativ_direction=-1, XX=32760, ZZ=32760, feed=-1, tool=0, inch=-1, gmcode=-1, hh=-1, code_type=0;
	uint8_t used_length=0, pos=0;
	uint8_t tx[SPI_MSG_LENGTH] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0};
	
	//User Input for Message
	printf("Message-Types:\n");
	printf("--------------\n");
	printf("000 Update Machine State\n");
	printf("001 Programm Start at Block\n");
	printf("002 Programm Stop\n");
	printf("003 Programm Pause\n");
	printf("004 Spindle on with RPM and Direction\n");
	printf("005 Spindle off\n");
	printf("006 Stepper on\n");
	printf("007 Stepper off\n");
	printf("008 X-Stepper move with feed\n");
	printf("009 Z-Stepper move with feed\n");
	printf("010 Set Tool-Position (and INIT)\n");
	printf("011 Origin-Offset\n");
	printf("012 metric or inch (maybe not needed)\n");
	printf("013 New CNC-Programm wit NN Blocks in metric or inch\n");
	printf("014 CNC-Code-Block\n");
	printf("015 shutdown\n");
	printf("016 Reset Errors\n\n");
	
	printf("Message-Type: ");
	if ((msg_type>=0) && (msg_type<=16)) printf("%i\n",msg_type);
	else do {
		scanf("%d",&msg_type);
		getchar();
	} while ((msg_type<0) || (msg_type>16));
	tx[pos++] = msg_type;
	
	switch (msg_type) {
		case 0:   	//Update Machine State
					break;
		case 1:   	//Programm Start at Block
					printf("Block-No (0 to 500): ");
					if ((block>=CNC_CODE_NMIN) && (block<=CNC_CODE_NMAX)) printf("%i\n",block);
					else do {
						scanf("%d",&block);
						getchar();
					} while ((block<CNC_CODE_NMIN) || (block>CNC_CODE_NMAX));
					tx[pos++] = block>>8;
					tx[pos++] = block;
					break;
		case 2: 	//Programm Stop
					break;
		case 3:   	//Programm Pause
					break;
		case 4:   	//Spindle on with RPM and Direction
					printf("RPM (460 to 3220): ");
					if ((rpm>=REVOLUTIONS_MIN) && (rpm<=REVOLUTIONS_MAX)) printf("%i\n",rpm);
					else do {
						scanf("%d",&rpm);
						getchar();
					} while ((rpm<REVOLUTIONS_MIN) || (rpm>REVOLUTIONS_MAX));
					tx[pos++] = rpm>>8;
					tx[pos++] = rpm;
					
					printf("Spindel direction invers (0 or 1): ");
					if ((spindle_direction>=0) && (spindle_direction<=1)) printf("%i\n",spindle_direction);
					else do {
						scanf("%d",&spindle_direction);
						getchar();
					} while ((spindle_direction<0) || (spindle_direction>1));
					tx[pos++] = spindle_direction;
					break;
		case 5:  	//Spindle off
					break;
		case 6:   	//Stepper on
					break;
		case 7:   	//Stepper off
					break;
		case 8:   	//X-Stepper move with feed
		case 9:   	//Z-Stepper move with feed
					printf("Feed (2 to 499): ");
					if ((feed>=F_MIN) && (feed<=F_MAX)) printf("%i\n",feed);
					else do {
						scanf("%d",&feed);
						getchar();
					} while ((feed<F_MIN) || (feed>F_MAX));
					tx[pos++] = feed>>8;
					tx[pos++] = feed;
					
					printf("negativ direction (0 or 1): ");
					if ((negativ_direction>=0) && (negativ_direction<=1)) printf("%i\n",negativ_direction);
					else do {
						scanf("%d",&negativ_direction);
						getchar();
					} while ((negativ_direction<0) || (negativ_direction>1));
					tx[pos++] = negativ_direction;
					break;
		case 10:   	//Set Tool-Position (and INIT)
					printf("X-Offset (+-5999): ");
					if ((XX>=-X_MIN_MAX_CNC) && (XX<=X_MIN_MAX_CNC)) printf("%i\n",XX); //does not work
					else do {
						scanf("%d",&XX);
						getchar();
					} while ((XX<-X_MIN_MAX_CNC) || (XX>X_MIN_MAX_CNC)); //does not work
					tx[pos++] = XX>>8;
					tx[pos++] = XX;
					
					printf("Z-Offset (+-32700): ");
					if ((ZZ>=-Z_MIN_MAX_CNC) && (ZZ<=Z_MIN_MAX_CNC)) printf("%i\n",ZZ); //does not work
					else do {
						scanf("%d",&ZZ);
						getchar();
					} while ((ZZ<-Z_MIN_MAX_CNC) || (ZZ>Z_MIN_MAX_CNC)); //does not work
					tx[pos++] = ZZ>>8;
					tx[pos++] = ZZ;
					
					printf("Tool (1 to 6): ");
					if ((tool>=T_MIN) && (tool<=T_MAX)) printf("%i\n",tool);
					else do {
						scanf("%d",&tool);
						getchar();
					} while ((tool<T_MIN) || (tool>T_MAX));
					tx[pos++] = tool;
					break;
		case 11:   	//Origin-Offset
					printf("X-Offset (+-5999): ");
					if ((XX>=-X_MIN_MAX_CNC) && (XX<=X_MIN_MAX_CNC)) printf("%i\n",XX); //does not work
					else do {
						scanf("%d",&XX);
						getchar();
					} while ((XX<-X_MIN_MAX_CNC) || (XX>X_MIN_MAX_CNC)); //does not work
					tx[pos++] = XX>>8;
					tx[pos++] = XX;
					
					printf("Z-Offset (+-32700): ");
					if ((ZZ>=-Z_MIN_MAX_CNC) && (ZZ<=Z_MIN_MAX_CNC)) printf("%i\n",ZZ); //does not work
					else do {
						scanf("%d",&ZZ);
						getchar();
					} while ((ZZ<-Z_MIN_MAX_CNC) || (ZZ>Z_MIN_MAX_CNC)); //does not work
					tx[pos++] = ZZ>>8;
					tx[pos++] = ZZ;
					break;
		case 12:  	//metric or inch (maybe not needed)
					printf("metric or inch (0 or 1): ");
					if ((inch>=0) && (inch<=1)) printf("%i\n",inch);
					else do {
						scanf("%d",&inch);
						getchar();
					} while ((inch<0) || (inch>1));
					tx[pos++] = inch;
					break;
		case 13:  	//New CNC-Programm wit N Blocks in metric or inch
					printf("Blocks (0 to 500): ");
					if ((block>=CNC_CODE_NMIN) && (block<=CNC_CODE_NMAX)) printf("%i\n",block);
					else do {
						scanf("%d",&block);
						getchar();
					} while ((block<CNC_CODE_NMIN) || (block>CNC_CODE_NMAX));
					tx[pos++] = block>>8;
					tx[pos++] = block;
					
					printf("metric or inch (0 or 1): ");
					if ((inch>=0) && (inch<=1)) printf("%i\n",inch);
					else do {
						scanf("%d",&inch);
						getchar();
					} while ((inch<0) || (inch>1));
					tx[pos++] = inch;
					break;
		case 14:  	//CNC-Code-Block
					printf("Block-No (0 to 500): ");
					if ((block>=CNC_CODE_NMIN) && (block<=CNC_CODE_NMAX)) printf("%i\n",block);
					else do {
						scanf("%d",&block);
						getchar();
					} while ((block<CNC_CODE_NMIN) || (block>CNC_CODE_NMAX));
					tx[pos++] = block>>8;
					tx[pos++] = block;
					
					printf("G- or M-Code (G or M): ");
					if ((code_type == 'G') || (code_type == 'M')) printf("%c\n",code_type);
					{
						scanf("%c",&code_type);
						getchar();
					} while ((code_type != 'G') && (code_type != 'M'));
					tx[pos++] = code_type;
					
					if (code_type == 'G') {
						printf("G-Code (0 to 196): ");
						if ((gmcode>=GM_CODE_MIN) && (gmcode<=G_CODE_MAX)) printf("%i\n",gmcode);
						else do {
							scanf("%d",&gmcode);
							getchar();
						} while ((gmcode<GM_CODE_MIN) || (gmcode>G_CODE_MAX));
					}
					else {
						printf("M-Code (0 to 99): ");
						if ((gmcode>=GM_CODE_MIN) && (gmcode<=M_CODE_MAX)) printf("%i\n",gmcode);
						else do {
							scanf("%d",&gmcode);
							getchar();
						} while ((gmcode<GM_CODE_MIN) || (gmcode>M_CODE_MAX));
					}
					tx[pos++] = gmcode;
					
					printf("X (+-5999): ");
					if ((XX>=-X_MIN_MAX_CNC) && (XX<=X_MIN_MAX_CNC)) printf("%i\n",XX);
					else do {
						scanf("%d",&XX);
						getchar();
					} while ((XX<-X_MIN_MAX_CNC) || (XX>X_MIN_MAX_CNC));
					scanf("%d",&XX);
					getchar();
					tx[pos++] = XX>>8;
					tx[pos++] = XX;
					
					printf("Z (+-32700): ");
					if ((ZZ>=-Z_MIN_MAX_CNC) && (ZZ<=Z_MIN_MAX_CNC)) printf("%i\n",ZZ);
					else do {
						scanf("%d",&ZZ);
						getchar();
					} while ((ZZ<-Z_MIN_MAX_CNC) || (ZZ>Z_MIN_MAX_CNC));
					tx[pos++] = ZZ>>8;
					tx[pos++] = ZZ;
					
					printf("Feed (2 to 499): ");
					if ((feed>=F_MIN) && (feed<=F_MAX)) printf("%i\n",feed);
					else do {
						scanf("%d",&feed);
						getchar();
					} while ((feed<F_MIN) || (feed>F_MAX));
					tx[pos++] = feed>>8;
					tx[pos++] = feed;
					
					printf("H (0 to 999): ");
					if ((hh>=H_MIN) && (hh<=H_MAX)) printf("%i\n",hh);
					else do {
						scanf("%d",&hh);
						getchar();
					} while ((hh<H_MIN) || (hh>H_MAX));
					tx[pos++] = hh>>8;
					tx[pos++] = hh;
					
					break;
		case 15:  	//Shutdown
					break;
		case 16:  	//Reset Errors
					break;
		default:  	//SPI-Error "PID unkown"
					break;
	}
	
	printf("\n");
	
	used_length = pos;
	
	//zero unused bytes
	for (pos; pos<(SPI_MSG_LENGTH-1); pos++) {
		tx[pos] = 0;
	}
	
	//CRC
	tx[pos] = CRC8(tx, used_length);
	
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

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");
	else { //output received Message
		/*
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
		*/
		//100 byte2=bit7_stepper|bit6_spindle_direction|bit5_spindle|bit4_inch|bit3_pause|bit2_manual|bit1_init|bit0_control_active RPM_H&L XX ZZ FF HH T NN ERROR_Numbers CRC-8 #Machine State
		printf("Incomming Message:\n");
		printf("------------------\n");
		printf("PID: %i\n", rx[0]);
		char active = (rx[1]>>STATE_CONTROL_ACTIVE_BIT)&1;
		printf("Control active: %i\n", active);
		char init = (rx[1]>>STATE_INIT_BIT)&1;
		printf("init: %i\n", init);
		char manual = (rx[1]>>STATE_MANUAL_BIT)&1;
		printf("manual: %i\n", manual);
		char pause = (rx[1]>>STATE_PAUSE_BIT)&1;
		printf("Pause: %i\n", pause);
		char inch = (rx[1]>>STATE_INCH_BIT)&1;
		printf("inch: %i\n", inch);
		char spindel_on = (rx[1]>>STATE_SPINDLE_BIT)&1;
		printf("Spindel on: %i\n", spindel_on);
		char spindel_direction = (rx[1]>>STATE_SPINDLE_DIRECTION_BIT)&1;
		printf("Spindel direction: %i\n", spindel_direction);
		char stepper_on = (rx[1]>>STATE_STEPPER_BIT)&1;
		printf("Stepper on: %i\n", stepper_on);
		rpm = (((int)rx[2]<<8)|(rx[3]));
		printf("RPM: %i\n", rpm);
		XX = (((int)rx[4]<<8)|(rx[5]));
		printf("X: %i\n", XX);
		ZZ = (((int)rx[6]<<8)|(rx[7]));
		printf("Z: %i\n", ZZ);
		feed = (((int)rx[8]<<8)|(rx[9]));
		printf("F: %i\n", feed);
		hh = (((int)rx[10]<<8)|(rx[11]));
		printf("H: %i\n", hh);
		tool = rx[12];
		printf("T: %i\n", tool);
		block = (((int)rx[13]<<8)|(rx[14]));
		printf("Block-No: %i\n", block);
		char spi_error = (rx[15]>>ERROR_SPI_BIT)&1;
		printf("SPI-Error: %i\n", spi_error);
		char cnc_code_error = (rx[15]>>ERROR_CNC_CODE_BIT)&1;
		printf("CNC-Code-Error: %i\n", cnc_code_error);
		char spindel_error = (rx[15]>>ERROR_SPINDLE_BIT)&1;
		printf("Spindel-Error: %i\n", spindel_error);
		char crc_in = rx[16];
		printf("CRC: %i\n\n", crc_in);
		
		//Ouptut to Machine-State-File
		machinestatefile = fopen(MACHINE_STATE_FILE, "w");
		if (fd < 0) printf("can't open Machine State file\n");
		else {
			fprintf(machinestatefile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
			fprintf(machinestatefile, "<machinestate>\n");
			fprintf(machinestatefile, "\t<state>\n");
			fprintf(machinestatefile, "\t\t<active>%i</active>\n", active);
			fprintf(machinestatefile, "\t\t<init>%i</init>\n", init);
			fprintf(machinestatefile, "\t\t<manual>%i</manual>\n", manual);
			fprintf(machinestatefile, "\t\t<pause>%i</pause>\n", pause);
			fprintf(machinestatefile, "\t\t<inch>%i</inch>\n", inch);
			fprintf(machinestatefile, "\t\t<spindel_on>%i</spindel_on>\n", spindel_on);
			fprintf(machinestatefile, "\t\t<spindel_direction>%i</spindel_direction>\n", spindel_direction);
			fprintf(machinestatefile, "\t\t<stepper_on>%i</stepper_on>\n", stepper_on);
			fprintf(machinestatefile, "\t<state>\n");
			fprintf(machinestatefile, "\t<measure>\n");
			fprintf(machinestatefile, "\t\t<rpm_measure>%i</rpm_measure>\n", rpm);
			fprintf(machinestatefile, "\t\t<x_actual>%i</x_actual>\n", XX);
			fprintf(machinestatefile, "\t\t<z_actual>%i</z_actual>\n", ZZ);
			fprintf(machinestatefile, "\t\t<f_actual>%i</f_actual>\n", feed);
			fprintf(machinestatefile, "\t\t<t_actual>%i</t_actual>\n", tool);	
			fprintf(machinestatefile, "\t</measure>\n");
			fprintf(machinestatefile, "\t<error>\n");
			fprintf(machinestatefile, "\t\t<spi_error>%i</spi_error>\n", spi_error);
			fprintf(machinestatefile, "\t\t<cnc_code_error>%i</cnc_code_error>\n", cnc_code_error);
			fprintf(machinestatefile, "\t\t<spindel_error>%i</spindel_error>\n", spindel_error);
			fprintf(machinestatefile, "\t</error>\n");
			fprintf(machinestatefile, "\t<cncblock>\n");
			fprintf(machinestatefile, "\t\t<n_actual>%i</n_actual>\n", block);
			fprintf(machinestatefile, "\t</cncblock>\n");
			fprintf(machinestatefile, "</machinestate>\n");

			printf("close(machinestatefile)\n");
			fclose(machinestatefile);
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
	     "  -3 --3wire    SI/SO signals shared\n");
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
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

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
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	
	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open SPI-device");
	
	// Register signal and signal handler
	signal(SIGINT, signal_callback_handler);

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	while(1) {
		transfer(fd);
	}
	
	//printf("close(fd)\n");
	//close(fd);

	return ret;
}
