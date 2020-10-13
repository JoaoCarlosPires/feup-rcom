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

#define TRANSMITTER 1
#define RECEIVER 0

/**
  * llopen function - starts the communication and proceed according to the type (send or receive)
  * @param porta COM1, COM2, ...
  * @param flag TRANSMITTER (1) or RECEIVER (0)
  * @return identificador da ligação de dados ou valor negativo em caso de erro
  */
int llopen(char* porta, int flag);
