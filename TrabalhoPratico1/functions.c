#include "functions.h"
#include "utils.h"

int llopen(char *porta, int flag)
{
	fd = open(porta, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		perror(porta);
		exit(-1);
	}

	if (flag == TRANSMITTER)
	{

		TRAMA_SET[0] = FLAG;
		TRAMA_SET[1] = A1;
		TRAMA_SET[2] = C_SET;
		TRAMA_SET[3] = A1 ^ C_SET;
		TRAMA_SET[4] = FLAG;

		if (tcgetattr(fd, &oldtio) == -1)
		{ /* save current port settings */
			perror("tcgetattr");
			exit(-1);
		}

		bzero(&newtio, sizeof(newtio));
		newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;

		/* set input mode (non-canonical, no echo,...) */
		newtio.c_lflag = 0;

		newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
		newtio.c_cc[VMIN] = 0;	 /* blocking read until 1 chars received */

		/* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

		tcflush(fd, TCIOFLUSH);

		if (tcsetattr(fd, TCSANOW, &newtio) == -1)
		{
			perror("tcsetattr");
			exit(-1);
		}

		printf("New termios structure\n");

		unsigned char buf;

		/* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */
		int CURR_STATE = START;
		UA_RCV = FALSE;

		do
		{
			
			write(fd, TRAMA_SET, sizeof(TRAMA_SET));
			alarm(TIMEOUT);
			alarm_active = FALSE;
			write(STDOUT_FILENO,"Trama SET sent\n",15);
			CURR_STATE = START;

			while (!UA_RCV && !alarm_active)
			{
				//buf = 0b01111110;
				read(fd, &buf, 1);
				CURR_STATE = stateMachine(CURR_STATE, &buf, C_UA,A1);
			}
	
		} while (alarm_active && allarms_called < 3);

		if (allarms_called == 3)
		{
			//return FALSE;
		}
		else
		{
			write(STDOUT_FILENO,"Trama UA received\n",18);
			allarms_called = 0;
			alarm_active = 0;
			//return TRUE;
		}
	}
	else if (flag == RECEIVER)
	{
		TRAMA_UA[0] = FLAG;
		TRAMA_UA[1] = A1;
		TRAMA_UA[2] = C_UA;
		TRAMA_UA[3] = A1 ^ C_UA;
		TRAMA_UA[4] = FLAG;

		if (tcgetattr(fd, &oldtio) == -1)
		{ /* save current port settings */
			perror("tcgetattr");
			exit(-1);
		}

		bzero(&newtio, sizeof(newtio));
		newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
		newtio.c_iflag = IGNPAR;
		newtio.c_oflag = 0;

		/* set input mode (non-canonical, no echo,...) */
		newtio.c_lflag = 0;

		newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
		newtio.c_cc[VMIN] = 0;	 /* blocking read until 1 chars received */

		/* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

		tcflush(fd, TCIOFLUSH);

		if (tcsetattr(fd, TCSANOW, &newtio) == -1)
		{
			perror("tcsetattr");
			exit(-1);
		}

		printf("New termios structure\n");
		
		unsigned char buf;
		STOP = FALSE;
		int curr_state = START;

		while (curr_state != FINISH)
		{ //receive control message

			read(fd, &buf, 1);
			curr_state = stateMachine(curr_state, &buf, C_SET,A1);
		}

		write(STDOUT_FILENO,"Trama SET received\n",19);
		write(fd, TRAMA_UA, sizeof(TRAMA_UA)); //send control UA message
		write(STDOUT_FILENO,"Trama UA sent\n",14);
	}

	if (allarms_called >= 3) {
		return FALSE;
	} else
		return TRUE;
}

int llwrite(int fd, unsigned char *buffer, int length)
{	
	unsigned char *TRAMA_DATA = malloc(length + 7);

	TRAMA_DATA[0] = FLAG;
	TRAMA_DATA[1] = A1;
	TRAMA_DATA[2] = C_SET;
	TRAMA_DATA[3] = A1 ^ C_SET;

	unsigned char * aux;
	aux = bcc_cal(buffer);

	buffer = stuffing(buffer, &length);

	for (int i = 0; i < length; i++) {
		TRAMA_DATA[4 + i] = buffer[i];
	}
	
	int len = length + 6;

	if (aux[1] != 0) {
		len++;
		TRAMA_DATA[4 + length] = aux[0];
		TRAMA_DATA[5 + length] = aux[1];
		TRAMA_DATA[6 + length] = TRAMA_DATA[0];
	} else {
		TRAMA_DATA[4 + length] = aux[0];
		TRAMA_DATA[5 + length] = TRAMA_DATA[0];
	}

	int CURR_STATE = START;
	unsigned char aux2;
	
	allarms_called = 0;

	do {
			
			write(fd, TRAMA_DATA, len);
			write(STDOUT_FILENO,"Trama DATA sent\n",17);
			alarm(TIMEOUT);
			alarm_active = FALSE;
			CURR_STATE = START;
			
			while ((CURR_STATE!=FINISH) && (!alarm_active))
			{
				read(fd, &aux2, 1);
			//	write(STDOUT_FILENO, "Stuck here2", 11);
				CURR_STATE = stateMachine(CURR_STATE, &aux2, C_RR, A1);
			}

			//write(STDOUT_FILENO, "After while\n", 13);

	} while (alarm_active && (allarms_called < 3));

	allarms_called = 0;

	return len;
}

unsigned char * llread(int fd, int * size)
{

	int CURR_STATE = START;
	int flag = 0;
	
	unsigned char ul;

	int mySize = *size;

	unsigned char * mensagem = (unsigned char *) malloc(0);

	while(flag == 0) {
		
		read(fd, &ul, 1);

		//counter++;

		switch (CURR_STATE)
		{

		case START:

			if (ul == FLAG){
				CURR_STATE = FLAG_RCV;
			
			}
			break;

		case FLAG_RCV:

			if (ul == A1){
				CURR_STATE = A_RCV;
				
			}
			else
				CURR_STATE = START;
			break;

		case A_RCV:

			if (ul == C_SET){ 
				CURR_STATE = C_RCV;
			
			}else
				CURR_STATE = START;
			break;

		case C_RCV:

			if (ul == (A1 ^ C_SET)){
				CURR_STATE = DATA;
				
			}
			else
				CURR_STATE = START;
			break;

		case DATA:
				
			if(ul == FLAG){
				mensagem = destuffing(mensagem, &mySize);
				flag = 1;
			}else{
				mensagem = (unsigned char *)realloc(mensagem,++mySize);
				mensagem[mySize-1] = ul;
			}
			break;
		}

	}

	unsigned char TRAMA_RR[5];

	TRAMA_RR[0] = FLAG;
	TRAMA_RR[1] = A1;
	TRAMA_RR[2] = C_RR;
	TRAMA_RR[3] = A1 ^ C_RR;
	TRAMA_RR[4] = FLAG;
	
	write(fd, TRAMA_RR, 5);

	write(STDOUT_FILENO,"Trama RR sent\n",14);

	*size = mySize;

	return mensagem;
}

int llclose(int fd, int flag)
{

	sleep(1);

	if (flag == TRANSMITTER)
	{

		TRAMA_DISC[0] = FLAG;
		TRAMA_DISC[1] = A1;
		TRAMA_DISC[2] = C_DISC;
		TRAMA_DISC[3] = A1 ^ C_DISC;
		TRAMA_DISC[4] = FLAG;
	
		TRAMA_UA[0] = FLAG;
		TRAMA_UA[1] = A2;
		TRAMA_UA[2] = C_UA;
		TRAMA_UA[3] = A2 ^ C_UA;
		TRAMA_UA[4] = FLAG;

		write(fd,TRAMA_DISC,sizeof(TRAMA_DISC));

		write(STDOUT_FILENO,"Trama DISC sent\n",16);

		int curr_state = START;
		unsigned char buf;

		while(curr_state != FINISH){
			
			read(fd,&buf,1);
			
			curr_state = stateMachine(curr_state,&buf,C_DISC,A2);

		}

		write(STDOUT_FILENO,"Trama DISC received\n",20);
		
		write(fd,TRAMA_UA,sizeof(TRAMA_UA));

		write(STDOUT_FILENO,"Trama UA sent\n",14);


	}

	else if (flag == RECEIVER)
	{
		TRAMA_DISC[0] = FLAG;
		TRAMA_DISC[1] = A2;
		TRAMA_DISC[2] = C_DISC;
		TRAMA_DISC[3] = A2 ^ C_DISC;
		TRAMA_DISC[4] = FLAG;

		int curr_state = START;
		unsigned char buf;

		while(curr_state != FINISH){
			
			read(fd,&buf,1);
			
			curr_state = stateMachine(curr_state,&buf,C_DISC,A1);

		}

		write(STDOUT_FILENO,"Trama DISC received\n",20);

		write(fd,TRAMA_DISC,sizeof(TRAMA_DISC));

		write(STDOUT_FILENO,"Trama DISC sent\n",16);
		
		curr_state = START;

		while(curr_state != FINISH){
			
			read(fd,&buf,1);
			
			curr_state = stateMachine(curr_state,&buf,C_UA,A2);

		}

		write(STDOUT_FILENO,"Trama UA received\n",18);

	}

	return 0;
}