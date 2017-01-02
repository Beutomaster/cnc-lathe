/* pipe_server.c */
// compile with: gcc -o pipe_server pipe_server.c
//test with second console: echo "test" >> /home/pi/spi_com/arduino_pipe.tx
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
//#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define BUF 4096
#define MSG_SUCCESS "Sending your message to Arduino!\0"
#define MSG_ERROR_STATE "Could not send message to Arduino at this Machine State!\0"
#define MSG_ERROR_CLIENT "Could not send message to Arduino, because it is in exclusively use by another Client!\0"
#define CLIENT_SESSION_ID_ARRAY_LENGTH 27
//example-sid: 15a1rgdq662cms5m9qe3f55n74

char state=0, client_sid[CLIENT_SESSION_ID_ARRAY_LENGTH], pid;
int exclusive = 0;

//static void send_to_arduino (const char *text, const char *client_sid) {
void send_to_arduino (const char *text) {
	sscanf(text,"%s\n%d", client_sid, &pid);
	//output message
	printf ("Message from Client-SESSION: %s\n", client_sid);
	printf ("Message-PID: %i\n", pid);
	printf ("Complete Message from Client-SESSION: %s\n", text);
}

int main (void) {
	char buffer[BUF], answer_to_client[BUF], answer_fifo_name[BUF];
	int r_fd, w_fd, i;
	//FILE *r_fz, *w_fz;

	//allow all rights for new created files
	umask(0);

	//Server creates arduino_pipe.tx, if it does not exist
	if (mkfifo ("arduino_pipe.tx", O_RDWR | 0666) < 0) {
	  //arduino_pipe.tx exists
	  if(errno == EEXIST)
		 printf ("arduino_pipe.tx exists, trying to use it!\n");
	  else {
		 perror("mkfifio()");
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
	r_fd = open ("arduino_pipe.tx", O_RDWR); //create an Filedescriptor for Low-Level I/O-Functions like read/write
	if (r_fd == -1) {
	  perror ("open(1)");
	  exit (EXIT_FAILURE);
	}
	//r_fz = fdopen(r_fd, "r"); //create an Filepointer for High-Level I/O-Functions like fscanf
	
	//set parameter for select
	fd_set r_fd_set;
	struct timeval timeout;
	int ret;

	while (1) { //Server-Loop
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
			printf("Updating Machine-State after waiting for message on pipe for 1s!\n");
		}
		else if (ret < 0) //error
		{
			printf("select returned with an error while waiting for message on pipe!\n");
		}
		else //there was activity on the file descripor
		{
			//printf("select says pipe is readable\n");
			//fscanf(r_fz,"%s\n%d", client_sid, &pid);
			
			if (read (r_fd, buffer, BUF) != 0) {
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
						perror ("open(2)");
						exit (EXIT_FAILURE);
					}
					write (w_fd, answer_to_client, strlen(answer_to_client));
					close (w_fd);
				}
				*/

				//process message
				send_to_arduino (buffer);
			}
		}
	}
	return EXIT_SUCCESS;
}