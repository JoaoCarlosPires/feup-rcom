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

unsigned char * buffer;

void alarmHandler()
{
	allarms_called++;
	alarm_active = TRUE;
	printf("Alarm Active\n");
}

/** 
 * processFile - prepara o array de caracteres que será enviado do emissor para o receptor, colocando-o na variável global *buffer*
 * @param filePath - path do ficheiro imagem a processar
 * @return retorna o tamanho do array de caracteres, tamanho este que será passado como argumento à função llwrite() 
*/
unsigned char* processFile(char* filePath, int *size) {

	int c = 1;
	int n = 0;

	FILE *fl = fopen(filePath, "rb");  
    fseek(fl, 0, SEEK_END);  
    long len = ftell(fl);  
    unsigned char * buffer = (unsigned char*)malloc(len);  
    fseek(fl, 0, SEEK_SET);  
    fread(buffer, 1, len, fl);  
    fclose(fl); 

//	char * aux = malloc(len*2);
//	int i = 0, aux_i = 0;

	/*
	while (aux_i != len) {
		if (data[aux_i] == 0b01111110) {
			aux[i] = 0x7d;
			aux[i+1] = 0x5e;
			i++;
		}
		else if (data[aux_i] == 0b01111101) {
			aux[i] = 0x7d;
			aux[i+1] = 0x5d;
			i++;	
		} else {
			aux[i] = data[aux_i];
		}
		i++;
		aux_i++;
	}*/

	

	n = len > 255 ? (len % 255) : len;

	int l2 = len / 256;
	int l1 = len % 256;

	char control[4] = {c, n, l2, l1};

	int total_size = len; //+ 4*sizeof(int);

	//buffer = malloc(total_size);

//	memcpy(buffer, control, 4*sizeof(int));
//	memcpy(buffer + 4, aux, len);

	//printf("This is the size of the image %i", total_size);
	*size = total_size;
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
	int fd, lenght;

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
		fd = llopen(porta, RECEIVER);

		// leitura dos pacotes de dados recebidos
		
		unsigned char *mensagem;
		
		int a = llread(fd, mensagem);

		llclose(fd,RECEIVER);

		/*
		if (createPicture(buffer,a) != 0) {
			perror("Unable to create picture\n");
			exit(-1);
		}*/

		// enviar confirmação
	} else { 
		// processamento da imagem
		
		unsigned char * buffer = processFile(imagem,&lenght); 

		if (lenght <= 0) {
			printf("Error in image processing\n");
			exit(1);
		}

		// abertura da comunicação no lado do trasmissor
		fd = llopen(porta, TRANSMITTER);

		// escrita através do transmissor dos pacotes de dados
		llwrite(fd, buffer, lenght);

		

		llclose(fd, TRANSMITTER);

		// receber confirmação
	}

	//close(fd);

	return 0;
}
