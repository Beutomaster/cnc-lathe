/* file_parser.c */
// compile with: gcc -o file_parser file_parser.c
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//SPI
#define SPI_BYTE_LENGTH_PRAEAMBEL 4
#define SPI_MSG_LENGTH (18+SPI_BYTE_LENGTH_PRAEAMBEL)

//File-Parser
#define LINELENGTH 80

static int cnc_code_array_length =0;

struct cnc_code_block {
	unsigned int N; //block-No.
	char GM; //G or M-Code
	unsigned char GM_NO; //G/M-Code-Number
	int XI; //X/I-Parameter
	int ZK; //Z/K-Parameter (K for M99)
	int FTLK; //F/T/L/K-Parameter (K for G33 and G78)
	int HS; //H/S-Parameter
};

int file_parser() {
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
	};

	FILE *cnc_code_file;
	cnc_code_file = fopen("/var/www/html/uploads/cnc_code.txt", "r");
	
	char c = 0, success=0;
	char line[LINELENGTH] = {};
	int n = 0, i = 0, j = 0, StartSignLine = 0, StopSignLine = 0;
	fpos_t startpos;
	
	cnc_code_array_length =0;
	
	//find start- and stop-signs '%'
	while (c != EOF){
		StopSignLine++; //stopsign not needed
		fgets(line, LINELENGTH, cnc_code_file);
		n = sscanf(line,"%c", &c);
		if (n < 1) {
			if (errno != 0) {
				perror("scanf");
				fclose(cnc_code_file);
				return EXIT_FAILURE;
			}
		}
		else if (!StartSignLine) {
			StartSignLine = StopSignLine;
			fgetpos(cnc_code_file,&startpos);
		}
		else break; //StopSignLine
	}
	//end if no cnc-code found between start- and stop-signs
	if (!StartSignLine || StopSignLine-StartSignLine<1) {
		fclose(cnc_code_file);
		return EXIT_FAILURE;
	}
	
	//reserve memory for cnc_code_array (should be global)
	cnc_code_array_length = StopSignLine-StartSignLine-1;
	static struct cnc_code_block cnc_code_array[cnc_code_array_length] = {0}; //zero it
	
	//rewind file-pointer to StartSignLine+1
	fsetpos(cnc_code_file,&startpos);
	
	//read cnc-code
	for (i=0; i<cnc_code_array_length; i++) {
		fgets(line, LINELENGTH, cnc_code_file);
		n = fscanf(line,"N%d %c%d %c%d %c%d %c%d %c%d", &cnc_code_block_raw.N, &cnc_code_block_raw.GM, &cnc_code_block_raw.GM_NO, &cnc_code_block_raw.c1, &cnc_code_block_raw.p1, &cnc_code_block_raw.c2, &cnc_code_block_raw.p2, &cnc_code_block_raw.c3, &cnc_code_block_raw.p3, &cnc_code_block_raw.c4, &cnc_code_block_raw.p4);
		if (n < 3) {
			if (errno != 0) perror("scanf");
			else perror("Parameter not matching");
			fclose(cnc_code_file);
			return EXIT_FAILURE;
		}
		//process Code-Line
		if (cnc_code_block_raw.GM == 'G') {
			switch (cnc_code_block_raw.GM_NO){
				case 0:
						break;
				default:
						perror("G-Code unkown");
						fclose(cnc_code_file);
						return EXIT_FAILURE;
			}
		}
		else if (cnc_code_block_raw.GM == 'M') {
			switch (cnc_code_block_raw.GM_NO){
				case 0:
						break;
				default:
						perror("M-Code unkown");
						fclose(cnc_code_file);
						return EXIT_FAILURE;
			}
		}
		else {
			perror("Parameter not matching");
			fclose(cnc_code_file);
			return EXIT_FAILURE;
		}
		cnc_code_array[i].N = cnc_code_block_raw.N;
		cnc_code_array[i].GM = cnc_code_block_raw.GM;
		cnc_code_array[i].GM_NO = cnc_code_block_raw.GM_NO;
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
				perror("L-Parameter target block does not exist");
				fclose(cnc_code_file);
				return EXIT_FAILURE;
			}
		}
	}
	
	fclose(cnc_code_file);
	return EXIT_SUCCESS;
}

static int spi_create_cnc_code_messages() {
	char msg_type = 16;
	unsigned int i, pos, used_length;
	
	for (i=0; i<cnc_code_array_length; i++) {
		pos=SPI_BYTE_LENGTH_PRAEAMBEL-1;
		tx[pos++] = msg_type;
		tx[pos++] = msg_number;
		tx[pos++] = (i+1)>>8;
		tx[pos++] = (i+1);
		tx[pos++] = cnc_code_array[i].GM;
		tx[pos++] = cnc_code_array[i].GM_NO;
		tx[pos++] = cnc_code_array[i].XI >> 8;
		tx[pos++] = cnc_code_array[i].XI;
		tx[pos++] = cnc_code_array[i].ZK >> 8;
		tx[pos++] = cnc_code_array[i].ZK;
		tx[pos++] = cnc_code_array[i].FTLK; //Problem L is int
		tx[pos++] = cnc_code_array[i].HS >> 8;
		tx[pos++] = cnc_code_array[i].HS;
		
		used_length = pos;
		
		//zero unused bytes (not needed in loop)
		for (pos; pos<(SPI_MSG_LENGTH-1); pos++) {
			tx[pos] = 0;
		}

		//CRC
		tx[pos] = CRC8(tx, SPI_BYTE_LENGTH_PRAEAMBEL, used_length);
		
		//send msg
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
	}
	return EXIT_SUCCESS;
}

int main (void) {
	if(!file_parser()) spi_create_cnc_code_messages(); //create and send messages from file

	return EXIT_SUCCESS;
}