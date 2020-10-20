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
#include <signal.h>

#define TRANSMITTER 1
#define RECEIVER 0

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4

#define TIMEOUT 3

int allarms_called = 0;
unsigned char TRAMA_SET[4];
struct termios oldtio, newtio;
int UA_RCV = 0;
int alarm_active = FALSE;

void alarmHandler()
{
    allarms_called++;
    alarm_active = TRUE;
}

/**
  * llopen function - starts the communication and proceed according to the type (send or receive)
  * @param porta COM1, COM2, ...
  * @param flag TRANSMITTER (1) or RECEIVER (0)
  * @return identificador da ligação de dados ou valor negativo em caso de erro
  */
int llopen(char* porta, int flag);
void stateMachine(int *curr_state, unsigned char *input);
