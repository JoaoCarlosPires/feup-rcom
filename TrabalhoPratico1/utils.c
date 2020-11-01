#include "utils.h"

void alarmHandler()
{
	allarms_called++;
	alarm_active = TRUE;
	printf("Alarm Active\n");
}

unsigned char * bcc_cal(unsigned char *buffer)
{
	unsigned char bcc = buffer[0];

	int len = sizeof(buffer);

	for (int i = 1; i < len; i++)
	{
		bcc ^= buffer[i];
	}

	unsigned char stuff_bcc[2];

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
		stuff_bcc[1] = 0;
	}

	unsigned char * aux = stuff_bcc;

	return aux;
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

unsigned char * stuffing(unsigned char * byteArray, int * len) {

	unsigned char * mensagem = malloc(*len);

	int trackI = 0;

	for (int i = 0; i < *len; i ++) {
		if (byteArray[i] == 0b01111110) {
			mensagem = realloc(mensagem, ++*len);
			mensagem[trackI] = 0x7d;
			mensagem[trackI+1] = 0x5e;
			trackI++;
			}
		else if (byteArray[i] == 0b01111101) {
			mensagem = realloc(mensagem, ++*len);
			mensagem[trackI] = 0x7d;
			mensagem[trackI+1] = 0x5d;
			trackI++;
		} else {
			mensagem[trackI] = byteArray[i];
		}
		trackI++;
	}

	printf("Array after stuffing %i\n", *len);

	return mensagem;
}

unsigned char * destuffing(unsigned char * byteArray, int * len) {
	
	unsigned char * mensagem = malloc(*len);

	int trackI = 0;

	int mySize = *len;

	printf("Array before destuffing %i\n", mySize);

	for (int i = 0; i < mySize; i++) {
		if (byteArray[i] == 0b1111101 && byteArray[i+1] == 0b1011110) {
			mensagem[trackI] = 0b01111110; 	
			mySize--;
			i++;
		} else if (byteArray[i] == 0b1111101 && byteArray[i+1] == 0b1011101) {
			mensagem[trackI] = 0b01111101;
			mySize--;
			i++; 
		} else {
			mensagem[trackI] = byteArray[i];
		}
		trackI++;
	}

	*len = mySize;

	return mensagem;
	
}
