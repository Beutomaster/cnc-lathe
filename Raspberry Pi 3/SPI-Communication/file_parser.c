/* file_parser.c */
// compile with: gcc -o file_parser file_parser.c
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

//development switch
#define FILEPARSER_STANDALONE

//SPI
#define SPI_BYTE_LENGTH_PRAEAMBEL 4
#define SPI_MSG_LENGTH (18+SPI_BYTE_LENGTH_PRAEAMBEL)

//temp globals
char msg_number=0;
int messages_notreceived=0;
uint8_t tx[SPI_MSG_LENGTH] = {0x7F,0xFF,0x7F,0xFF, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0};

//File-Parser
#define CNC_CODE_FILE "/var/www/html/uploads/cnc_code.txt"
#define LINELENGTH 80

//globals
FILE *cnc_code_file;
int cnc_code_array_length = 0;

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


//functions
int test_value_range(char name, int value, int min, int max) {
	//test if range of value matches
	if (value < min || value > max) {
		fprintf(stderr, "Backend File-Parser: %c out of Range\n", name);
		return 0;
	}
	return 1;
}
	
int get_next_cnc_code_parameter(char *InputParameterName, int *InputParameter, char name, int *OutputValue, char optional, int min, int max) {
	int success = 1;
	//check Name of Parameter
	if (*InputParameterName == name) {
		//test range of value matches
		if (success = test_value_range(name, *InputParameter, min, max)) {
			OutputValue=InputParameter;
		}
		//else return 0;
	}
	else {
		OutputValue=0; //set Default Value
		if (!optional) {
			fprintf(stderr, "no %c-Parameter or incorrect format", name);
			success = 0;
			//return 0;
		} //else return 1;
	}
	return success;
	//return 2; //Parameter found
}

int file_parser_abort() {
	fclose(cnc_code_file);
	return EXIT_FAILURE;
}

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
	} cnc_code_block_raw;

	cnc_code_file = fopen(CNC_CODE_FILE, "r");
	
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
		else if (!StartSignLine) {
			StartSignLine = StopSignLine;
			fgetpos(cnc_code_file,&startpos);
		}
		else break; //StopSignLine
	}
	//end if no cnc-code found between start- and stop-signs
	if (!StartSignLine || StopSignLine-StartSignLine<1) {
		return file_parser_abort();
	}
	
	//rewind file-pointer to StartSignLine+1
	fsetpos(cnc_code_file,&startpos);
	
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
	
	//read cnc-code
	for (i=0; i<cnc_code_array_length; i++) {
		fgets(line, LINELENGTH, cnc_code_file);
		n = sscanf(line,"N%d %c%d %c%d %c%d %c%d %c%d", &cnc_code_block_raw.N, &cnc_code_block_raw.GM, &cnc_code_block_raw.GM_NO, &cnc_code_block_raw.c1, &cnc_code_block_raw.p1, &cnc_code_block_raw.c2, &cnc_code_block_raw.p2, &cnc_code_block_raw.c3, &cnc_code_block_raw.p3, &cnc_code_block_raw.c4, &cnc_code_block_raw.p4);
		if (n < 3) {
			if (errno != 0) perror("Backend File-Parser: scanf");
			else fprintf(stderr, "Backend File-Parser: Parameter not matching\n");
			return file_parser_abort();
		}
		
		ret = 1;
		i=0;
		InputParameterName[0] = &cnc_code_block_raw.c1;
		InputParameter[0] = &cnc_code_block_raw.p1;
		InputParameterName[1] = &cnc_code_block_raw.c2;
		InputParameter[1] = &cnc_code_block_raw.p2;
		InputParameterName[2] = &cnc_code_block_raw.c3;
		InputParameter[2] = &cnc_code_block_raw.p3;
		InputParameterName[3] = &cnc_code_block_raw.c4;
		InputParameter[4] = &cnc_code_block_raw.p4;
							
		//process Code-Line
		if (cnc_code_block_raw.GM == 'G') {
			switch (cnc_code_block_raw.GM_NO){
				case 0:
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						if (!ret) {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						break;
				case 1:
				case 2:
				case 3:
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						if (ret) i++;
						else {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						break;
				case 4:
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 0, 0, X_DWELL_MIN_MAX_CNC); //Ranges correct?
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
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'L', &cnc_code_array[i].FTLK, 0, CNC_CODE_NMIN, CNC_CODE_NMAX);
						if (!ret) return file_parser_abort();
						break;
				case 26: //G26(X,Z,T);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'T', &cnc_code_array[i].FTLK, 0, T_MIN, T_MAX);
						if (!ret) return file_parser_abort();
						break;
				case 27: //G27(L); //Jump
						/*
						if (get_next_cnc_code_parameter($Parameter, $line, $N, "L", $L, 0, CNC_CODE_NMIN, CNC_CODE_NMAX)) $jumps[$N] = $L;
						else $success = 0;
						*/
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'L', &cnc_code_array[i].FTLK, 0, CNC_CODE_NMIN, CNC_CODE_NMAX);
						if (!ret) return file_parser_abort();
						break;
				case 33: //G33(Z,K);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'K', &cnc_code_array[i].ZK, 0, IK_MIN, K_MAX); //Ranges correct?
						if (!ret) return file_parser_abort();
						break;
				case 64:
						break;
				case 73: //G73(Z,F);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						break;
				case 78: //G78(X,Z,K,H);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'K', &cnc_code_array[i].FTLK, 0, IK_MIN, K_MAX); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'H', &cnc_code_array[i].HS, 0, H_MIN, H_MAX); //Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 81: //G81(Z,F);
				case 82: //G82(Z,F);
				case 83: //G83(Z,F);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 84: //G84(X,Z,F,H);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'H', &cnc_code_array[i].HS, 0, H_MIN, H_MAX); //optional?, Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 85: //G85(Z,F);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 86: //G86(X,Z,F,H);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'H', &cnc_code_array[i].HS, 0, H_G86_MIN, H_MAX); //optional?, Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 88: //G88(X,Z,F,H);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'H', &cnc_code_array[i].HS, 0, H_MIN, H_MAX); //optional?, Ranges correct?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 89: //G89(Z,F);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].XI, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (ret) i++;
						else return file_parser_abort();
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'F', &cnc_code_array[i].FTLK, 0, F_MIN, F_MAX); //optional?
						if (!ret) return file_parser_abort();
						//Ranges correct?
						break;
				case 90:
				case 91:
						break;
				case 92: //G92(X,Z);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						break;
				case 94:
				case 95:
				case 96:
						break;
				case 97: //G97(S);
				case 196: //G196(S);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'S', &cnc_code_array[i].HS, 1, REVOLUTIONS_MIN, REVOLUTIONS_MAX); //optional?
						if (!ret) return file_parser_abort();
						break;
				default:
						fprintf(stderr, "Backend File-Parser: G-Code is not supported\n");
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
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC);
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC);
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'T', &cnc_code_array[i].FTLK, 0, T_MIN, T_MAX);
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
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 1, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 1, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) {
							fprintf(stderr, "No X- or Z-Parameter or incorrect format. At least one of them is required.\n");
							return file_parser_abort();
						}
						break;
				case 99: //M99(I,K);
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'X', &cnc_code_array[i].XI, 0, -X_MIN_MAX_CNC, X_MIN_MAX_CNC); //optional?
						if (ret) i++;
						ret &= get_next_cnc_code_parameter(InputParameterName[i], InputParameter[i], 'Z', &cnc_code_array[i].ZK, 0, -Z_MIN_MAX_CNC, Z_MIN_MAX_CNC); //optional?
						if (!ret) return file_parser_abort();
						break;
				default:
						fprintf(stderr, "Backend File-Parser: M-Code is not supported\n");
						return file_parser_abort();
			}
		}
		else {
			fprintf(stderr, "Backend File-Parser: Parameter not matching\n");
			return file_parser_abort();
		}
		if (cnc_code_block_raw.N >= CNC_CODE_NMIN && cnc_code_block_raw.N <= CNC_CODE_NMAX) {
			cnc_code_array[i].N = cnc_code_block_raw.N;
		}
		else {
			fprintf(stderr, "Backend File-Parser: N out of Range\n");
			return file_parser_abort();
		}	
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
				fprintf(stderr, "Backend File-Parser: L-Parameter target block does not exist\n");
				return file_parser_abort();
			}
		}
	}
	
	return file_parser_abort();
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
		
		#ifndef FILEPARSER_STANDALONE
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
		#else
			tx[pos] = 0; //CRC
			
			//Output
			pos=SPI_BYTE_LENGTH_PRAEAMBEL-1;
			printf("%i ", tx[pos++]); //msg_type
			printf("%i ", tx[pos++]); //msg_number;
			printf("%i ", tx[pos++]); //(i+1)>>8;
			printf("%i ", tx[pos++]); //(i+1);
			printf("%i ", tx[pos++]); //cnc_code_array[i].GM;
			printf("%i ", tx[pos++]); //cnc_code_array[i].GM_NO;
			printf("%i ", tx[pos++]); //cnc_code_array[i].XI >> 8;
			printf("%i ", tx[pos++]); //cnc_code_array[i].XI;
			printf("%i ", tx[pos++]); //cnc_code_array[i].ZK >> 8;
			printf("%i ", tx[pos++]); //cnc_code_array[i].ZK;
			printf("%i ", tx[pos++]); //cnc_code_array[i].FTLK; //Problem L is int
			printf("%i ", tx[pos++]); //cnc_code_array[i].HS >> 8;
			printf("%i \n", tx[pos++]); //cnc_code_array[i].HS;
		#endif
	}
	return EXIT_SUCCESS;
}

int main (void) {
	if(!file_parser()) spi_create_cnc_code_messages(); //create and send messages from file

	return EXIT_SUCCESS;
}