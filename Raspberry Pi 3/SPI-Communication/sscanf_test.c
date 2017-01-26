#include <stdio.h>
#include <errno.h>

int n,p0,p1;
char s[5]={0,0,0,0,0};
char buffer[2][8]= 	{
						{'1','1','1','1','\n','2','\n','\0'},
						{'t','e','s','t','\n','4','\n','\0'},
					};

void test(const char *buffer_local) {
	printf ("Address of buffer_local: %i\n", buffer_local);
	n = sscanf(buffer_local,"%s\n%d\n", s, &p1);
	if (n != 2) {
		if (errno != 0) perror("scanf");
		else fprintf(stderr, "Parameter not matching\n");
	}
	printf ("[0-3]: %s\n", s);
	printf ("[5]: %i\n", p1);
	printf ("Address of buffer_local: %i\n", buffer_local);
	n = sscanf(buffer_local,"%s\n%d\n", s, &p1);
	if (n != 2) {
		if (errno != 0) perror("scanf");
		else fprintf(stderr, "Parameter not matching\n");
	}
	printf ("[0-3]: %s\n", s);
	printf ("[5]: %i\n", p1);
	printf ("Address of buffer_local: %i\n", buffer_local);
}

void main() {
	printf ("Address of buffer: %i\n", buffer);
	test(buffer[1]);
}