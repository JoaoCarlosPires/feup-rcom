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

#include "functions.h"
#include "utils.h"

/** 
 * processFile - prepara o array de caracteres que será enviado do emissor para o receptor, colocando-o na variável global *buffer*
 * @param filePath - path do ficheiro imagem a processar
 * @return retorna o tamanho do array de caracteres, tamanho este que será passado como argumento à função llwrite() 
*/
unsigned char* processFile(char* filePath, int *size) {

	FILE *fl = fopen(filePath, "rb");  
    fseek(fl, 0, SEEK_END);  
    long len = ftell(fl);  
    unsigned char * buffer = (unsigned char*)malloc(len);  
    fseek(fl, 0, SEEK_SET);  
    fread(buffer, 1, len, fl);  
    fclose(fl); 

	*size = len;
	return buffer;
}

int createPicture(unsigned char * pictureBuffer,int size) {
	FILE* picture; 
    picture = fopen("picture.gif", "wb+"); 

	fwrite(pictureBuffer, 1, size-1, picture);
	fclose(picture);

	return 0;
}


int main(int argc, char** argv) {

	alarm_active = 0;

	(void)signal(SIGALRM, alarmHandler);
	
	char * porta = NULL;
	char * imagem = NULL;
	int lenght;

	(void)signal(SIGALRM, alarmHandler);

	// Se argc = 2 -> nome do programa e porta de série (receptor)
	// Se argc = 3 -> nome do programa, porta de série e path da imagem (emissor)
    if (argc < 2 || argc > 3) {
		printf("Usage:\tInvalid number of arguments\n");
      	exit(1);
	}

	// Com o uso deste ciclo, permite-se que os argumentos sejam passados
	// sem nenhuma ordem em específico
	for (int i = 1; i < argc ; i++) {
		if ((strcmp("/dev/ttyS10", argv[i])==0) || (strcmp("/dev/ttyS11", argv[i])==0))
			porta = argv[i];
		else {
			int result = access(argv[i], F_OK);
			if (result == 0) {
				imagem = argv[i];
			} else {
				printf("Usage:\tInvalid arguments\n");
				exit(1);
			}
		}
	}

	if (imagem == NULL) {
		// abertura da comunicação no lado do receptor
		if (llopen(porta, RECEIVER)) {
		
			// leitura dos pacotes de dados recebidos
			
			unsigned char *mensagem;
			int pictureSize = 0;
			
			mensagem = llread(fd, &pictureSize);

			llclose(fd,RECEIVER);

			/*for (int i = 0; i < pictureSize; i++) {
				printf("%u\n", mensagem[i]);
			} */		

			printf("Size after destuffing %i\n", pictureSize);

			if (createPicture(mensagem, pictureSize) != 0) {
				perror("Unable to create picture\n");
				exit(-1);
			}
		}

	} else { 
		// abertura da comunicação no lado do trasmissor
		if (llopen(porta, TRANSMITTER)) {

			// processamento da imagem		
			unsigned char * buffer = processFile(imagem,&lenght); 

			/*for (int i = 0; i < lenght; i++) {
				printf("%u\n", buffer[i]);
			} */

			if (lenght <= 0) {
				printf("Error in image processing\n");
				exit(1);
			}

			printf("Original size of picture %i\n", lenght);

			// escrita através do transmissor dos pacotes de dados
			llwrite(fd, buffer, lenght);

			llclose(fd, TRANSMITTER);

		}

	}

	if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
			perror("tcsetattr");
			exit(-1);
	}

	close(fd);

	return 0;
}
