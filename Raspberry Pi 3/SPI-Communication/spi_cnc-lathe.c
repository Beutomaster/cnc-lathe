/*
 * SPI testing utility (using spidev driver)
 *
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

 //defines
#define SPI_MSG_LENGTH 17
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

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

static void pabort(const char *s)
{
	perror(s);
	abort();
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
	int ret, block=-1, rpm=-1, msg_type=-1, spindle_direction=-1, negativ_direction=-1, XX=0, ZZ=0, feed=-1, tool=0, inch=-1, gcode=-1, hh=-1, code_type=0;
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
	
	while ((msg_type<0) || (msg_type>16)) {
		printf("Message-Type: ");
		scanf("%d",&msg_type);
	}
	tx[pos++] = msg_type;
	
	switch (msg_type) {
		case 0:   	//Update Machine State
					break;
		case 1:   	//Programm Start at Block
					while ((block<0) || (block>500)) {
						printf("Block-No (0 to 500): ");
						scanf("%d",&block);
					}
					tx[pos++] = block<<8;
					tx[pos++] = block;
					break;
		case 2: 	//Programm Stop
					break;
		case 3:   	//Programm Pause
					break;
		case 4:   	//Spindle on with RPM and Direction
					while ((rpm<0) || (rpm>3220)) {
						printf("RPM (0 to 3220): ");
						scanf("%d",&rpm);
					}
					tx[pos++] = rpm<<8;
					tx[pos++] = rpm;
					
					while ((spindle_direction<0) || (spindle_direction>1)) {
						printf("Spindel direction invers (0 or 1): ");
						scanf("%d",&spindle_direction);
					}
					tx[pos++] = spindle_direction;
					break;
		case 5:  	//Spindle off
					break;
		case 6:   	//Stepper on
					break;
		case 7:   	//Stepper off
					break;
		case 8:   	//X-Stepper move with feed
					while ((feed<0) || (feed>499)) {
						printf("Feed (0 to 499): ");
						scanf("%d",&feed);
					}
					tx[pos++] = feed<<8;
					tx[pos++] = feed;
					
					while ((negativ_direction<0) || (negativ_direction>1)) {
						printf("negativ direction (0 or 1): ");
						scanf("%d",&negativ_direction);
					}
					tx[pos++] = negativ_direction;
					break;
		case 9:   	//Z-Stepper move with feed
					while ((feed<0) || (feed>499)) {
						printf("Feed (0 to 499): ");
						scanf("%d",&feed);
					}
					tx[pos++] = feed<<8;
					tx[pos++] = feed;
					
					while ((negativ_direction<0) || (negativ_direction>1)) {
						printf("negativ direction (0 or 1): ");
						scanf("%d",&negativ_direction);
					}
					tx[pos++] = negativ_direction;
					break;
		case 10:   	//Set Tool-Position (and INIT)
					printf("X-Offset: ");
					scanf("%d",&XX);
					tx[pos++] = XX<<8;
					tx[pos++] = XX;
					
					printf("Z-Offset: ");
					scanf("%d",&ZZ);
					tx[pos++] = ZZ<<8;
					tx[pos++] = ZZ;
					
					while ((tool<1) || (tool>6)) {
						printf("Tool (1 to 6): ");
						scanf("%d",&tool);
					}
					tx[pos++] = tool;
					break;
		case 11:   	//Origin-Offset
					printf("X-Offset: ");
					scanf("%d",&XX);
					tx[pos++] = XX<<8;
					tx[pos++] = XX;
					
					printf("Z-Offset: ");
					scanf("%d",&ZZ);
					tx[pos++] = ZZ<<8;
					tx[pos++] = ZZ;
					break;
		case 12:  	//metric or inch (maybe not needed)
					while ((inch<0) || (inch>1)) {
						printf("metric or inch (0 or 1): ");
						scanf("%d",&inch);
					}
					tx[pos++] = inch;
					break;
		case 13:  	//New CNC-Programm wit N Blocks in metric or inch
					while ((block<0) || (block>500)) {
						printf("Blocks (0 to 500): ");
						scanf("%d",&block);
					}
					tx[pos++] = block<<8;
					tx[pos++] = block;
					
					while ((inch<0) || (inch>1)) {
						printf("metric or inch (0 or 1): ");
						scanf("%d",&inch);
					}
					tx[pos++] = inch;
					break;
		case 14:  	//CNC-Code-Block
					while ((block<0) || (block>500)) {
						printf("Block-No (0 to 500): ");
						scanf("%d",&block);
					}
					tx[pos++] = block<<8;
					tx[pos++] = block;
					
					while ((code_type != 'G') && (code_type != 'M')) {
						printf("G- or M-Code (G or M): ");
						scanf("%c",&code_type);
					}
					tx[pos++] = code_type;
					
					while ((gcode<0) || (gcode>200)) {
						printf("G/M-Code (0 to 200): ");
						scanf("%d",&gcode);
					}
					tx[pos++] = gcode;
					
					printf("X: ");
					scanf("%d",&XX);
					tx[pos++] = XX<<8;
					tx[pos++] = XX;
					
					printf("Z: ");
					scanf("%d",&ZZ);
					tx[pos++] = ZZ<<8;
					tx[pos++] = ZZ;
					
					while ((feed<0) || (feed>499)) {
						printf("Feed (0 to 499): ");
						scanf("%d",&feed);
					}
					tx[pos++] = feed<<8;
					tx[pos++] = feed;
					
					while ((hh<0) || (hh>32000)) {
						printf("H (0 to 32000): ");
						scanf("%d",&hh);
					}
					tx[pos++] = hh<<8;
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
		printf("Control active: %i\n", (rx[1]>>STATE_CONTROL_ACTIVE_BIT)&1);
		printf("init: %i\n", (rx[1]>>STATE_INIT_BIT)&1);
		printf("manual: %i\n", (rx[1]>>STATE_MANUAL_BIT)&1);
		printf("Pause: %i\n", (rx[1]>>STATE_PAUSE_BIT)&1);
		printf("inch: %i\n", (rx[1]>>STATE_INCH_BIT)&1);
		printf("Spindel on: %i\n", (rx[1]>>STATE_SPINDLE_BIT)&1);
		printf("Spindel direction: %i\n", (rx[1]>>STATE_SPINDLE_DIRECTION_BIT)&1);
		printf("Stepper on: %i\n", (rx[1]>>STATE_STEPPER_BIT)&1);
		printf("RPM: %i\n", (((int)rx[2]<<8)|(rx[3])));
		printf("X: %i\n", (((int)rx[4]<<8)|(rx[5])));
		printf("Z: %i\n", (((int)rx[6]<<8)|(rx[7])));
		printf("F: %i\n", (((int)rx[8]<<8)|(rx[9])));
		printf("H: %i\n", (((int)rx[10]<<8)|(rx[11])));
		printf("T: %i\n", rx[12]);
		printf("Block-No: %i\n", (((int)rx[13]<<8)|(rx[14])));
		printf("SPI-Error: %i\n", (rx[15]>>ERROR_SPI_BIT)&1);
		printf("CNC-Code-Error: %i\n", (rx[15]>>ERROR_CNC_CODE_BIT)&1);
		printf("Spindel-Error: %i\n", (rx[15]>>ERROR_SPINDLE_BIT)&1);
		printf("CRC: %i\n\n", rx[16]);
	}

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
	int fd;

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

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

	transfer(fd);

	close(fd);

	return ret;
}
