#include "functions.h"

void alarmHandler()
{
	allarms_called++;
	alarm_active = TRUE;
}

unsigned char bcc_cal(char *buffer)
{
	unsigned char bcc = 0;

	int len = sizeof(buffer);

	for (int i = 0; i < len; i++)
	{
		bcc ^= buffer[i];
	}

	if (bcc == 0b01111110)
	{
		bcc = 0x7d;
		bcc += 0x5e;
	}
	else if (bcc == 0b01111101)
	{
		bcc = 0x7d;
		bcc += 0x5d;
	}

	return bcc;
}

int llopen(char *porta, int flag)
{

	(void)signal(SIGALRM, alarmHandler);

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

		newtio.c_cc[VTIME] = 10; /* inter-character timer unused */
		newtio.c_cc[VMIN] = 1;	 /* blocking read until 1 chars received */

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

		printf("New termios structure set\n");

		unsigned char buf;

		//write(fd, TRAMA_SET, sizeof(TRAMA_SET));

		/* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */
		int CURR_STATE = START;

		do
		{
			alarm_active = FALSE;
			write(fd, TRAMA_SET, sizeof(TRAMA_SET));
			alarm(TIMEOUT);

			while (!UA_RCV && !alarm_active)
			{
				read(fd, &buf, 1);
				stateMachine(&CURR_STATE, &buf, C_UA,A1);
			}

		} while (!UA_RCV && allarms_called < 3);

		if (allarms_called == 3)
		{
			//return FALSE;
		}
		else
		{
			allarms_called = 0;
			alarm_active = FALSE;
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

		newtio.c_cc[VTIME] = 10; /* inter-character timer unused */
		newtio.c_cc[VMIN] = 1;	 /* blocking read until 1 chars received */

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

		printf("New termios structure set\n");

		unsigned char buf;
		STOP = FALSE;
		int curr_state = START;

		while (STOP == FALSE)
		{ //receive control message

			read(fd, &buf, 1);
			stateMachine(&curr_state, &buf, C_SET,A1);
		}

		write(fd, TRAMA_UA, sizeof(TRAMA_UA)); //send control UA message
	}

	return fd;
}

int llwrite(int fd, char *buffer, int length)
{

	unsigned char *TRAMA_DATA = malloc(sizeof(buffer) + 7);

	int C = C_SET; // provisorio para nao haver erros, deve ser corrigido

	TRAMA_DATA[0] = FLAG;
	TRAMA_DATA[1] = A1;
	TRAMA_DATA[2] = C;
	TRAMA_DATA[3] = A1 ^ C;

	for (int i = 0; i < length; i++)
	{
		TRAMA_DATA[4 + i] = buffer[i];
	}

	TRAMA_DATA[4 + length] = bcc_cal(buffer);
	if (sizeof(TRAMA_DATA[4 + length]) == 0x02)
		TRAMA_DATA[4 + length + 2] = TRAMA_DATA[0];
	else
		TRAMA_DATA[4 + length + 1] = TRAMA_DATA[0];

	write(fd, TRAMA_DATA, sizeof(TRAMA_DATA));

	return sizeof(TRAMA_DATA);
}

int llread(int fd, char *buffer)
{

	int CURR_STATE = START;
	int i = 0;
	int len = sizeof(buffer);

	int picture = creat("picture.gif", 0666);

	int C = C_SET; // provisorio para nao haver erros, deve ser corrigido

	do
	{

		read(fd, &buffer[i], 1);

		switch (CURR_STATE)
		{

		case START:

			if (buffer[i] == FLAG)
				CURR_STATE = FLAG_RCV;
			break;

		case FLAG_RCV:

			if (buffer[i] == A1)
				CURR_STATE = A_RCV;
			else
				CURR_STATE = START;
			break;

		case A_RCV:

			if (buffer[i]) // atençao falta completa
				CURR_STATE = C_RCV;
			else
				CURR_STATE = START;
			break;

		case C_RCV:

			if (buffer[i] == (A1 ^ C))
				CURR_STATE = BCC_RCV;
			else
				CURR_STATE = START;
			break;

		case BCC_RCV:

			if (buffer[i] == FLAG)
			{
				alarm(0);
				UA_RCV = 1;
			}
			else
			{
				write(picture, &buffer[i], 1);
			}
			break;

		default:
			break;
		}

		i++;
	} while (i != len);

	return 0;
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

		int curr_state = START;
		unsigned char buf;

		while(curr_state != FINISH){
			
			read(fd,&buf,1);
			
			stateMachine(&curr_state,&buf,C_DISC,A2);

		}

		write(fd,TRAMA_UA,sizeof(TRAMA_UA));


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
			
			stateMachine(&curr_state,&buf,C_DISC,A1);

		}

		write(fd,TRAMA_DISC,sizeof(TRAMA_DISC));

		curr_state = START;

		while(curr_state != FINISH){
			
			read(fd,&buf,1);
			
			stateMachine(&curr_state,&buf,C_UA,A2);

		}

	}

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
	{
		perror("tcsetattr");
		exit(-1);
	}

	close(fd);
	return 0;
}

void stateMachine(int *curr_state, unsigned char *input, int C, int A)
{

	switch (*curr_state)
	{

	case START:

		if (*input == FLAG)
			*curr_state = FLAG_RCV;
		break;

	case FLAG_RCV:

		if (*input == A)
			*curr_state = A_RCV;
		else
			*curr_state = START;
		break;

	case A_RCV:

		if (*input == C)
			*curr_state = C_RCV;
		else
			*curr_state = START;
		break;

	case C_RCV:

		if (*input == (A ^ C))
			*curr_state = BCC_RCV;
		else
			*curr_state = START;
		break;

	case BCC_RCV:

		if (*input == FLAG)
		{
			STOP = TRUE;
			alarm(0);
			UA_RCV = 1;
			*curr_state = FINISH;
		}
		break;
	}
}
