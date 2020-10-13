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

int main(int argc, char** argv) {
	
	// argv[1] será o path do ficheiro a transmitir
	// argv[2] será a porta de série

    if (argc < 3) {
		printf("Usage:\tInvalid number of arguments\n");
      	exit(1);
	}

	if ( (strcmp("/dev/ttyS0", argv[2])!=0) && 
  	     (strcmp("/dev/ttyS1", argv[2])!=0) ) {
		printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
		exit(1);
	}

	if (argv[1]==0) {
      printf("Usage:\tFile to transmit is not defined\n");
      exit(1);
    }

	return 0;
}
