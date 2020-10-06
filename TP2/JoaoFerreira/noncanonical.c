/*Non-Canonical Input Processing*/

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4

unsigned char TRAMA_SET[4];

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
	
	unsigned char buf;

	TRAMA_SET[0] = (unsigned char)01111110;
    TRAMA_SET[1] = 00000011;
    TRAMA_SET[2] = 00000011;
    TRAMA_SET[3] = TRAMA_SET[1]^TRAMA_SET[2];
	
	int CURR_STATE = START;
	
	    

	if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 10;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");



    while (STOP==FALSE) {       /* loop for input */
      
      res = read(fd,&buf,255);   /* returns after 1 char have been input */
      
	  switch(CURR_STATE){
		
		case START:
			
			if(strcmp(&buf, &TRAMA_SET[0])==0){
				CURR_STATE = FLAG_RCV;
			}else{

			}
			break;
		
		case FLAG_RCV:
		
			if(strcmp(&buf,&TRAMA_SET[1]) == 0){
				CURR_STATE = A_RCV;
			}else{
				CURR_STATE = START;
			}

			break;
	
		case A_RCV:
		
			if(strcmp(&buf,&TRAMA_SET[2]) == 0){
				CURR_STATE = C_RCV;
			}else{
				CURR_STATE = START;
			}
			
			break;
		
		case C_RCV:
		
			if(strcmp(&buf,&TRAMA_SET[3])==0){
				CURR_STATE = BCC_RCV;	
			}else{
			
				CURR_STATE = START;				
			
			}		
				
			break;
	
		case BCC_RCV:
		
			if(strcmp(&buf,&TRAMA_SET[0]) == 0){
				STOP = TRUE;		
			}	
		
			break;
	 
              					/* so we can printf... */
     
     // if (buf[res-1]=='\0') STOP=TRUE;
    	}
	}
   // res = write(fd, buf, res);

  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */

    sleep(1);

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}
