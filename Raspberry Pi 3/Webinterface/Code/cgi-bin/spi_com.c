#include <stdio.h>                // Needed for cgi
#include <stdlib.h>
#include <string.h>
#define MAX_PAARE 255
#include <fcntl.h>                // Needed for SPI port
#include <sys/ioctl.h>            // Needed for SPI port
#include <linux/spi/spidev.h>     // Needed for SPI port
#include <stdint.h>
//#include <wiringPiSPI.h>

//SPI
static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
int ret=0, fd, length;
unsigned char *data_string;

//CGI
char *getdata();
char *Strdup(const char *);
char *appendnewline(const char *);
void hex2ascii(char *);
char convert(char *);
struct CGI_DATEN *erstellen(char *);
void printf_error(char *);

struct CGI_DATEN {
   char *variable;
   char *wert;
   struct CGI_DATEN *next;
};

struct CGI_DATEN *ende = NULL;


//SPI Send and Receive
int SpiWriteRead (int fd, unsigned char *data, int length)
/* Schreiben und Lesen auf SPI. Parameter:
 * fd        Devicehandle
 * data      Puffer mit Sendedaten, wird mit Empfangsdaten überschrieben
 * length    Länge des Puffers
*/

  {
	struct spi_ioc_transfer spi[length]; /* Bibliotheksstruktur fuer Schreiben/Lesen */
	int i, ret;                          /* Zaehler, Returnwert */

	
/* Device oeffen */
	if ((fd = open(device, O_RDWR)) < 0)
	  {
	  perror("Fehler Open Device");
	  exit(1);
	  }
	
	/* Mode setzen */
ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
if (ret < 0)
  {
  perror("Fehler Set SPI-Modus");
  exit(1);
  }
  
  /* Mode abfragen */
ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
if (ret < 0)
  {
  perror("Fehler Get SPI-Modus");
  exit(1);
  }
  
  /* Wortlaenge setzen */
ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
if (ret < 0)
  {
  perror("Fehler Set Wortlaenge");
  exit(1);
  }
   
  /* Wortlaenge abfragen */
  ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
  if (ret < 0)
    {
    perror("Fehler Get Wortlaenge");
    exit(1);
    }

	/* Datenrate setzen */
ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
if (ret < 0)
  {
  perror("Fehler Set Speed");
  exit(1);
  }
   
  /* Datenrate abfragen */
  ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
  if (ret < 0)
    {
    perror("Fehler Get Speed");
    exit(1);
    }
	
  /* Daten uebergeben */
	for (i = 0; i < length; i++)
	  {
		spi[i].tx_buf        = (unsigned long)(data + i); // transmit from "data"
		spi[i].rx_buf        = (unsigned long)(data + i); // receive into "data"
		spi[i].len           = sizeof(*(data + i));
		spi[i].delay_usecs   = 0;
		spi[i].speed_hz      = speed;
		spi[i].bits_per_word = bits;
		spi[i].cs_change     = 0;
	  }

	printf("<p>Test1 Senden/Empfangen - ioctl</p>"); //Debugging
	  
	//ret = ioctl(fd, SPI_IOC_MESSAGE(length), &spi); //funzt net
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &spi);
	
	//echo -ne "\x0A" > /dev/spidev0.0
	if(ret < 0)
    {
		printf("<p>Test2 Senden/Empfangen - ioctl</p>"); //Debugging
		perror("Fehler beim Senden/Empfangen - ioctl");
		exit(1);
    }
	
	printf("<p>Test3 Senden/Empfangen - ioctl</p>"); //Debugging
	
	close(fd);
	
	return ret;
  }
  
  /*
 *  Funktion liest Daten in der POST- oder GET-Methode ein.
 *  Rückgabewert: String puffer mit den Daten
 *  bei Fehler  : NULL
 */
char *getdata(void) {
   unsigned long size;
   char *puffer = NULL;
   char *request = getenv("REQUEST_METHOD");
   char *cont_len;
   char *cgi_string;

   /* zuerst die Request-Methode überprüfen */
   if(  NULL == request )
      return NULL;
   else if( strcmp(request, "GET") == 0 ) {
      /* Die Methode GET -> Query-String abholen */
      cgi_string = getenv("QUERY_STRING");
      if( NULL == cgi_string )
         return NULL;
      else {
         puffer = (char *) Strdup(cgi_string);
         return puffer; /* Rückgabewert an den Aufrufer */
      }
   }
   else if( strcmp(request, "POST") == 0 ) {
      /* die Methode POST -> Länge des Strings
       * ermitteln (CONTENT_LENGTH) */
      cont_len = getenv("CONTENT_LENGTH");
      if( NULL == cont_len)
         return NULL;
      else {
         /* String CONTENT_LENGTH in unsigned long umwandeln */
         size = (unsigned long) atoi(cont_len);
         if(size <= 0)
            return NULL; /* Keine Eingabe!?!? */
      }
      /* jetzt lesen wir die Daten von stdin ein */
      puffer =(char *) malloc(size+1);
      if( NULL == puffer )
         return NULL;
      else {
         if( NULL == fgets(puffer, size+1, stdin) ) {
            free(puffer);
            return NULL;
         }
         else  /* Rückgabewerte an den Aufrufer */
            return puffer;
      }
   }

   /* Weder die GET-Methode noch die POST-Methode wurden verwendet. */
   else
      return NULL;
}

/*  Da die Funktion strdup() in der Headerdatei <string.h> keine
 *  ANSI-C-Funktion ist, schreiben wir eine eigene.
 */
char *Strdup(const char *str) {
   char *p;
   if(NULL == str)
      return NULL;
   else {
      p = (char *)malloc(strlen(str)+1);
      if(NULL == p)
         return NULL;
      else
         strcpy(p, str);
   }
   return p;
}

char *appendnewline(const char *str) {
   char *p;
   char n[1]="\n";
   if(NULL == str)
      return NULL;
   else {
      p = (char *)malloc(strlen(str)+2);
      if(NULL == p)
         return NULL;
      else
         strcat(p, n);
   }
   return p;
}

/* Wandelt einzelne Hexzeichen (%xx) in ASCII-Zeichen
 * und kodierte Leerzeichen (+) in echte Leerzeichen um. */
void hex2ascii(char *str) {
   int x,y;

   for(x=0,y=0; str[y] != '\0'; ++x,++y) {
      str[x] = str[y];
      /* Ein hexadezimales Zeichen? */
      if(str[x] == '%')  {
         str[x] = convert(&str[y+1]);
         y += 2;
      }
      /* Ein Leerzeichen? */
      else if( str[x] == '+')
         str[x]=' ';
   }
   /* geparsten String sauber terminieren */
   str[x] = '\0';
}




/* Funktion konvertiert einen String von zwei hexadezimalen
 * Zeichen und gibt das einzelne dafür stehende Zeichen zurück.
 */
char convert(char *hex) {
   char ascii;

   /* erster Hexawert */
   ascii =
   (hex[0] >= 'A' ? ((hex[0] & 0xdf) - 'A')+10 : (hex[0] - '0'));
   ascii <<= 4; /* Bitverschiebung schneller als ascii*=16 */
   /* zweiter Hexawert */
   ascii +=
   (hex[1] >= 'A' ? ((hex[1] & 0xdf) - 'A')+10 : (hex[1] - '0'));
   return ascii;
}

/* Liste aus Variable/Wert-Paaren erstellen
 * Rückgabewert: Anfangsadresse der Liste
 * Bei Fehler: NULL
 */
struct CGI_DATEN *erstellen(char *str) {
   char* s;
   char* res;
   /* Irgendwo gibt es auch eine Grenze, hier sind
      MAX_PAARE erlaubt. */
   char *paare[MAX_PAARE];
   struct CGI_DATEN *ptr_daten = NULL;
   struct CGI_DATEN *ptr_anfang = NULL;
   int i=0, j=0;

   /* Zuerst werden die Variablen/Werte-Paare anhand des Zeichens
    * '&' getrennt, sofern es mehrere sind. */
    s=str;
    res=strtok(s,"&");
    while( res != NULL && i < MAX_PAARE) {
       /* Wert von res dynamisch in char **pair speichern */
       paare[i] = (char *)malloc(strlen(res)+1);
       if(paare[i] == NULL)
          return NULL;
       paare[i] = res;
       res=strtok(NULL,"&");
       i++;
    }


   /* Jetzt werden die Variablen von den Werten getrennt und
    * an die Struktur CGI_DATEN übergeben. */
   while ( i > j )  { /* Das erste Element? */
      if(ptr_anfang == NULL) {
         ptr_anfang =(struct CGI_DATEN *)
           malloc(sizeof (struct CGI_DATEN *));
         if( ptr_anfang == NULL )
            return NULL;
         res = strtok( paare[j], "=");
         if(res == NULL)
            return NULL;
         ptr_anfang->variable = (char *)
           malloc(strlen(res)+1);
         if( ptr_anfang->variable == NULL )
            return NULL;
         ptr_anfang->variable = res;
         res = strtok(NULL, "\0");
         if(res == NULL)
            return NULL;
         ptr_anfang->wert = (char *) malloc(strlen(res)+1);
         if( ptr_anfang->wert == NULL )
            return NULL;
         ptr_anfang->wert = res;
         /* printf("%s %s<br>",
          * ptr_anfang->variable, ptr_anfang->wert); */
         ptr_anfang->next =(struct CGI_DATEN *)
           malloc(sizeof (struct CGI_DATEN *));
         if(ptr_anfang->next == NULL)
            return NULL;
         ptr_daten = ptr_anfang->next;
         j++;
      }
      else { /* die restlichen Elemente */
         res = strtok( paare[j], "=");
         if(res == NULL)
            return NULL;
         ptr_daten->variable =(char *)
           malloc(strlen(res)+1);
         if(ptr_daten->variable == NULL)
            return NULL;
         ptr_daten->variable = res;
         res = strtok(NULL, "\0");
         if(res == NULL)
            return NULL;
         ptr_daten->wert =(char *) malloc(strlen(res)+1);
         if(ptr_daten->wert == NULL)
            return NULL;
         ptr_daten->wert = res;
         /* printf("%s %s<br>",
          * ptr_daten->variable,  ptr_daten->wert); */
         ptr_daten->next = (struct CGI_DATEN *)
           malloc(sizeof (struct CGI_DATEN *));
         if( ptr_daten->next == NULL )
            return NULL;
         ptr_daten = ptr_daten->next;
         j++;
      }
   }
   ende = ptr_daten;
   /* Anfangsadresse der Liste struct CGI_DATEN zurückgeben */
   return ptr_anfang;
}

void loeschen(struct CGI_DATEN *daten) {
   struct CGI_DATEN *next = NULL;

   while(daten != ende) {
      next = daten->next;
      if(daten->variable != NULL)
         free(daten);
      daten=next;
   }
}

void printf_error(char *str) {
   printf("Content-Type: text/html\n\n");
   printf("<html><head>\n");
   printf("<title>CGI-Fehlermeldung</title>\n");
   printf("</head><body>\n");
   printf("%s",str);
   printf("</body></html>\n");
}
  
int main(int argc, char *argv[])
{
	char *str;
	char *str2;
	struct CGI_DATEN *cgi;
	struct CGI_DATEN *free_cgi;
	

	/* Eingabe einlesen */
   str = getdata();
   if(str == NULL) {
      printf_error("Fehler beim Einlesen von der "
                   "Formulareingabe");
      return EXIT_FAILURE;
   }
   
    /* Hexzeichen in ASCII-Zeichen konvertieren und aus '+'
    * Leerzeichen machen */
	hex2ascii(str);
   
   	/* Kontrollausgabe */
	printf("Content-type: text/html\n\n"); 
	printf("<html><body>\n"); /* \n isn't needed, but makes source more readable */ 
	printf("<p>"); /* <P> is an HTML paragraph tag */ 
	printf("SPI-Device.....: %s<br />", device);
	printf("SPI-Mode.......: %d<br />", mode);
	printf("Wortlaenge.....: %d<br />", bits);
	printf("Geschwindigkeit: %d Hz (%d kHz)<br />", speed, speed/1000);
	printf("%s",str);
	printf("</p>\n"); /* <P> is an HTML paragraph tag */
   
   //String senden
   str2 = appendnewline(str);
   SpiWriteRead (fd, (unsigned char *) str2, strlen(str2)); //es kommt kein zeichen an
   //SpiWriteRead (fd, (unsigned char *) appendnewline(str), (strlen(str)+1)); //funzt net, length muss const sein
   //mit WiringPi
   //int wiringPiSPISetup (int channel, int speed);
   //int wiringPiSPIDataRW (0, (unsigned char *) appendnewline(str), (strlen(str)+1));
   printf("<p>Test Senden/Empfangen - ioctl</p>"); //Debugging
   
   /* Liste der Formualardaten erstellen */
   cgi = erstellen(str);
   free_cgi = cgi;
   if (cgi == NULL) {
      printf_error("Fehler beim Erstellen der "
                   "Variablen/Werte-Liste\n");
      return EXIT_FAILURE;
   }
	
	/*
	//noch nicht fertig:
	//Sende-String erstellen
    while(cgi != NULL) {
	  data_string=cgi->variable;
	  data_string=cgi->wert;
      cgi = cgi->next;
    }
   
	SpiWriteRead (fd, data_string, length);
	*/
	
	//html schliessen
	printf("</body></html>\n");
	
	/* Speicher wieder freigeben */
	loeschen(free_cgi);
	
	return EXIT_SUCCESS;
}
