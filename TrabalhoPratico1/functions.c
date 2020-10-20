#include "functions.h"

int llopen(char* porta, int flag) {
	int fd;

	fd = open(porta, O_RDWR | O_NOCTTY );
    if (fd <0) {perror(porta); exit(-1); }

	if (flag == TRANSMITTER) {
		//call writenoncanonical code
	}
	else if (flag == RECEIVER) {
		//call noncanonical code
	}

	return fd;
}


