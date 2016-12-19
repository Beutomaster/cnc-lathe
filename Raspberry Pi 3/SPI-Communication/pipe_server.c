/* pipe_server.c */
//test with second console: echo "test" >> /home/pi/spi_com/arduino_pipe.tx
#include <stdio.h>
#include <unistd.h>
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

char state=0;
int exclusive = 0;

static void send_to_arduino (const char *text, int client_sid) {
   //output message
   printf ("Message from Client-SESSION: %i\n", client_sid);
   printf ("Message %s\n", text);
   return;
}

int main (void) {
   char buffer[BUF], answer_to_client[BUF], answer_fifo_name[BUF], client_sid[27];
   int r_fd, w_fd, n, i;
   
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
   r_fd = open ("arduino_pipe.tx", O_RDONLY);
   if (r_fd == -1) {
      perror ("open(1)");
      exit (EXIT_FAILURE);
   }
   
   while (1) { //Server-Loop
      if (read (r_fd, buffer, BUF) != 0) {
		/*
		//get SESSION-ID of calling client
		n = 0, i = 0;
		while (buffer[n] != '\n')
		client_sid[i++] = buffer[n++];
		pid[++i] = '\n';

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
      sleep(1);
   }
   return EXIT_SUCCESS;
}