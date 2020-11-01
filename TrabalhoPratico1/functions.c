#include "functions.h"



unsigned char bcc_cal(unsigned char *buffer)
{
	unsigned char bcc;

	int len = sizeof(buffer);

	for (int i = 0; i < len; i++)
	{
		bcc ^= buffer[i];
	}

	/*
	char stuff_bcc[2];

	if (bcc == 0b01111110)
	{
		stuff_bcc[0] = 0x7d;
		stuff_bcc[1] = 0x5e;
	}
	else if (bcc == 0b01111101)
	{
		stuff_bcc[0] = 0x7d;
		stuff_bcc[1] = 0x5d;
	} else {
		stuff_bcc[0] = bcc;
	}*/

	//char * aux = stuff_bcc;

	return bcc;
}

int llopen(char *porta, int flag)
{

	

	int fd;

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

	return fd;
}

int llwrite(int fd, unsigned char *buffer, int length)
{	

	unsigned char *TRAMA_DATA = malloc(length + 7);

	TRAMA_DATA[0] = FLAG;
	TRAMA_DATA[1] = A1;
	TRAMA_DATA[2] = C_SET;
	TRAMA_DATA[3] = A1 ^ C_SET;


	unsigned char aux;
	//aux = malloc(2*sizeof(char));
	aux = bcc_cal(buffer);

	TRAMA_DATA[4+length] = aux;
	TRAMA_DATA[5+length] = FLAG;
	/*
	if (sizeof(aux) == 2) {
		TRAMA_DATA[4 + length] = aux[0];
		TRAMA_DATA[5 + length] = aux[1];
		TRAMA_DATA[6 + length] = TRAMA_DATA[0];
	} else {
		TRAMA_DATA[4 + length] = aux[0];
		TRAMA_DATA[5 + length] = TRAMA_DATA[0];
	}*/

	int CURR_STATE = START;
	unsigned char aux2;
	
	allarms_called = 0;



	do {
			
			write(fd, TRAMA_DATA, sizeof(TRAMA_DATA));
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

		
//	write(fd, TRAMA_DATA, sizeof(TRAMA_DATA));

	
	
	
	allarms_called = 0;



	return sizeof(TRAMA_DATA);
}

int llread(int fd, unsigned char *mensagem)
{

	int CURR_STATE = START;
	//int i = 0;
	int flag = 0;
	
//	int picture = creat("picture.gif", 0666);
	unsigned char ul;
	
	mensagem = (unsigned char *) malloc(0);
	int mensagem_size = 0;

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
				flag = 1;
				
			}else{
				mensagem = (unsigned char *)realloc(mensagem,++mensagem_size);
				mensagem[mensagem_size-1] = ul;
				
				
				
			}
			break;
				
				/*
				write(STDOUT_FILENO, "teste", 6);
				if (ul == FLAG) {
					flag = 1;
					write(STDOUT_FILENO, "test1", 6);
				} else {
					write(picture, &buffer[i], 1);
					buffer = realloc(buffer, sizeof(buffer)+1);
					i++;
				}*/
				/*buffer[i] = ul;
				aux2 = bcc_cal(buffer);
				if (sizeof(bcc_cal(buffer)) == 1) {
					if (ul == aux2[0]) {
						CURR_STATE = BCC2_RCV;
					}
				} else {
					if (aux2[0] == ul) {
						read(fd, &ul, 1);
						if (aux2[1] == ul) {
							counter++;
							CURR_STATE = BCC2_RCV;
						}
					} else {
						write(picture, &buffer[i], 1);
						buffer = realloc(buffer, sizeof(buffer)+1);
						i++;
					}
				}
				*/

		
			
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

	return mensagem_size;
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

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
	{
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
	return 0;
}

int stateMachine(int curr_state, unsigned char *input, int C, int A)
{

	switch (curr_state)
	{

	case START:

		if (*input == FLAG)
			curr_state = FLAG_RCV;
		break;

	case FLAG_RCV:

		if (*input == A)
			curr_state = A_RCV;
		else
			curr_state = START;
		break;

	case A_RCV:

		if (*input == C)
			curr_state = C_RCV;
		else
			curr_state = START;
		break;

	case C_RCV:

		if (*input == (A ^ C))
			curr_state = BCC_RCV;
		else
			curr_state = START;
		break;

	case BCC_RCV:

		if (*input == FLAG)
		{
			STOP = TRUE;
			alarm(0);
			UA_RCV = 1;
			curr_state = FINISH;
		}else{
			curr_state = START;
		}
		break;
	

	default:
		break;
	
	}
	
	return curr_state;

}
