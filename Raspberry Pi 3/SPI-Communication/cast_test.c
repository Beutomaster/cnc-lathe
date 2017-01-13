//cast test
//gcc -o cast_test cast_test.c


#include <stdio.h>

signed char SCHAR = -125;
char CHAR2;
char CHAR = -125;
unsigned char UCHAR;
int INT = -1000;
unsigned int UINT;

void main () {
	CHAR2 = (char)SCHAR;
	printf ("SCHAR: %i\n", SCHAR);
	printf ("SCHAR: 0x%x\n", SCHAR);
	printf ("CHAR2: 0x%x\n", CHAR2);
	printf ("CHAR2: %i\n", CHAR2);
	UCHAR = (unsigned char)CHAR;
	printf (" CHAR: %i\n", CHAR);
	printf (" CHAR: 0x%x\n", CHAR);
	printf ("UCHAR: 0x%x\n", UCHAR);
	printf ("UCHAR: %i\n", UCHAR);
	UINT = (unsigned int)INT;
	printf (" INT: %i\n", INT);
	printf (" INT: 0x%x\n", INT);
	printf ("UINT: 0x%x\n", UINT);
	CHAR = (char)INT;
	UCHAR = (unsigned char)INT;
	printf (" INT: %i\n", INT);
	printf (" INT: 0x%x\n", INT);
	printf ("UCHAR: 0x%x\n", UCHAR);
	printf ("UCHAR: %i\n", UCHAR);
}