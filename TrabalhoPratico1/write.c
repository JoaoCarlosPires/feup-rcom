#include "write.h"

int allarms_called = 0;
//volatile int STOP = FALSE;
unsigned char TRAMA_SET[4];
struct termios oldtio, newtio;

int UA_RCV = 0;
int alarm_active = FALSE;

void alarmHandler()
{
    allarms_called++;
    alarm_active = TRUE;
}

int main(int argc, char **argv)
{

    int fd;

    TRAMA_SET[0] = (unsigned char)01111110;
    TRAMA_SET[1] = 00000011;
    TRAMA_SET[2] = 00000011;
    TRAMA_SET[3] = TRAMA_SET[1] ^ TRAMA_SET[2];

    if ((argc < 2) ||
        ((strcmp("/dev/ttyS0", argv[1]) != 0) &&
         (strcmp("/dev/ttyS1", argv[1]) != 0)))
    {
        printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
        exit(1);
    }

    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(argv[1], O_RDWR | O_NOCTTY);
    if (fd < 0)
    {
        perror(argv[1]);
        exit(-1);
    }

    (void)signal(SIGALRM, alarmHandler);

    if (!llopen(fd))
    {
        return -1; //se não tem sucesso o programa acaba
    }

    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(-1);
    }

    //aqui vêm as proximas funcoes;
    close(fd);
    return 0;
}

int llopen(int fd)
{

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
    newtio.c_cc[VMIN] = 1;   /* blocking read until 1 chars received */

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

    /*    
	int size = strlen(gets(buf));
    
    if (buf[size] != '\0')
        perror("Invalid string");
    */

    unsigned char buf;

    write(fd, TRAMA_SET, sizeof(TRAMA_SET));

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
            stateMachine(&CURR_STATE, &buf);
        }

    } while (!UA_RCV && allarms_called < 3);

    if (allarms_called == 3)
    {
        return FALSE;
    }
    else
    {
        allarms_called = 0;
        alarm_active = FALSE;
        return TRUE;
    }
}

int llwrite(){
    
    return 0;
    // to do
} 


void stateMachine(int *curr_state, unsigned char *input)
{

    switch (*curr_state)
    {

    case START:

        if (*input == TRAMA_SET[0])
        {
            *curr_state = FLAG_RCV;
        }
        break;

    case FLAG_RCV:

        if (*input == TRAMA_SET[1])
        {
            *curr_state = A_RCV;
        }
        else
        {
            *curr_state = START;
        }
        break;

    case A_RCV:
        if (*input == TRAMA_SET[2])
        {
            *curr_state = C_RCV;
        }
        else
        {
            *curr_state = START;
        }
        break;

    case C_RCV:

        if (*input == TRAMA_SET[3])
        {
            *curr_state = BCC_RCV;
        }
        else
        {
            *curr_state = START;
        }

        break;

    case BCC_RCV:

        if (*input == TRAMA_SET[0])
        {
            //STOP = TRUE;
            alarm(0);
            UA_RCV = 1;
        }

        break;
    }
}