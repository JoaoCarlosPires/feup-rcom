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

	*size = len+4;

	unsigned char * control = (unsigned char*)malloc(4+len);
	control[0] = C_Data;	// C
	control[1] = 0;			// N
	control[2] = len / 255;	// L2
	control[3] = len % 255; // L1

	for (int i = 0; i < len; i++) {
		control[4+i] = buffer[i];
	}

	return control;
}

int createPicture(unsigned char * pictureBuffer) {
	FILE* picture; 
    picture = fopen("picture.gif", "wb+"); 

	int pictureSize = pictureBuffer[2]*255 + pictureBuffer[3];
	unsigned char * auxiliar = malloc(pictureSize);
	for (int i = 0; i < pictureSize; i++) {
		auxiliar[i] = pictureBuffer[i+4];
	}

	fwrite(auxiliar, 1, pictureSize-1, picture);
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
			unsigned char *mensagem_start;
			unsigned char *mensagem_end;
			
			mensagem_start = llread(fd);

			if (mensagem_start[0] == C_Begin) {
				write(STDOUT_FILENO, "Received start message\n", 23);
				mensagem = llread(fd);
				if (mensagem[0] != ESCAPE) {
					mensagem_end = llread(fd);
					if (mensagem_end[0] == C_End) {
						write(STDOUT_FILENO, "Received end message\n", 21);
					}	
				}  
			}
			
			llclose(fd,RECEIVER);		

			if (createPicture(mensagem) != 0) {
				perror("Unable to create picture\n");
				exit(-1);
			}
		}

	} else if (llopen(porta, TRANSMITTER)) { // abertura da comunicação no lado do trasmissor

			// processamento da imagem		
			unsigned char * buffer = processFile(imagem,&lenght); 

			if (lenght <= 5) { // Exigimos que a imagem tenha pelo menos 1 byte. Os outros 4 são os do cabeçalho
				printf("Error in image processing\n");
				exit(1);
			}

			unsigned char * control = malloc(7);
			control[0] = C_Begin; 		//C
			control[1] = T_FileSize;	//T1
			control[2] = 1;				//L1
			control[3] = lenght;		//V1

			// escrita através do transmissor dos pacotes de dados
			if (llwrite(fd, control, 4))
				if (llwrite(fd, buffer, lenght)) {
					control[0] = C_End;			//C
					llwrite(fd, control, 4);
				}
						
			llclose(fd, TRANSMITTER);

	}


	if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
			perror("tcsetattr");
			exit(-1);
	}

	close(fd);

	return 0;
}

