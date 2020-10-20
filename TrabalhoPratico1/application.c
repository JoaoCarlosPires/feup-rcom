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

char * buffer;

/** 
 * processFile - prepara o array de caracteres que será enviado do emissor para o receptor, colocando-o na variável global *buffer*
 * @param filePath - path do ficheiro imagem a processar
 * @return retorna o tamanho do array de caracteres, tamanho este que será passado como argumento à função llwrite() 
*/
int processFile(char* filePath) {
	return 0;
}

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

	// abertura da comunicação no lado do trasmissor
	// int fd1 = llopen(argv[2], 1);

	// abertura da comunicação no lado do receptor
	// int fd2 = llopen(argv[2], 0);

	/*
	if (fd1 != fd2) {
		printf("Erro no estabelecimento de ligação");
		exit(0);
	}

	int fd = fd1;
	*/

	// processamento da imagem
	int lenght = processFile(argv[1]); 
	if (lenght == -1) {
		printf("Error in image processing\n");
		exit(1);
	}

	// escrita através do transmissor dos pacotes de dados
	// llwrite(fd, buffer, lenght)

	// leitura dos pacotes de dados recebidos
	// llread(fd, buffer)

	// confirmação

	// llclose(fd);
	
	return 0;
}
