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

char * buffer;

/** 
 * processFile - prepara o array de caracteres que será enviado do emissor para o receptor, colocando-o na variável global *buffer*
 * @param filePath - path do ficheiro imagem a processar
 * @return retorna o tamanho do array de caracteres, tamanho este que será passado como argumento à função llwrite() 
*/
int processFile(char* filePath) {

	int c = 1;
	int n = 0;

	FILE *fl = fopen(filePath, "r");  
    fseek(fl, 0, SEEK_END);  
    long len = ftell(fl);  
    char * data = malloc(len);  
    fseek(fl, 0, SEEK_SET);  
    fread(data, 1, len, fl);  
    fclose(fl); 

	char * aux = malloc(len*2);
	int i = 0, aux_i = 0;

	while (aux_i != len) {
		if (data[aux_i] == 01111110) {
			aux[i] = 0x7d;
			aux[i+1] = 0x5e;
			i++;
		}
		else if (data[aux_i] == 01111101) {
			aux[i] = 0x7d;
			aux[i+1] = 0x5d;
			i++;	
		} else {
			aux[i] = data[aux_i];
		}
		i++;
		aux_i++;
	}

	len = sizeof(aux);

	n = len > 255 ? (len % 255) : len;

	int l2 = len / 256;
	int l1 = len % 256;

	char control[4] = {c, n, l2, l1};

	int total_size = len + 4*sizeof(int);

	buffer = malloc(total_size);

	memcpy(buffer, control, 4*sizeof(int));
	memcpy(buffer + 4, aux, len);

	return total_size;
}

int main(int argc, char** argv) {
	
	char * porta = NULL;
	char * imagem = NULL;
	int fd, lenght;

	// Se argc = 2 -> nome do programa e porta de série (receptor)
	// Se argc = 3 -> nome do programa, porta de série e path da imagem (emissor)
    if (argc < 2 || argc > 3) {
		printf("Usage:\tInvalid number of arguments\n");
      	exit(1);
	}

	// Com o uso deste ciclo, permite-se que os argumentos sejam passados
	// sem nenhuma ordem em específico
	for (int i = 1; i < argc ; i++) {
		if ((strcmp("/dev/ttyS0", argv[i])==0) || (strcmp("/dev/ttyS1", argv[i])==0))
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
		llread(fd, buffer);

		// enviar confirmação
	} else { 
		// processamento da imagem
		lenght = processFile(imagem); 
		if (lenght <= 0) {
			printf("Error in image processing\n");
			exit(1);
		}

		// abertura da comunicação no lado do trasmissor
		fd = llopen(porta, TRANSMITTER);
		
		// escrita através do transmissor dos pacotes de dados
		llwrite(fd, buffer, lenght);

		// receber confirmação
	}

	llclose(fd);
	
	return 0;
}
