#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#pragma once

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

#define TIMEOUT 5

unsigned char TRAMA_SET[5];
unsigned char TRAMA_UA[5];
unsigned char TRAMA_DISC[5];
struct termios oldtio, newtio;
int fd;

/**
  * inicia a comunicação e procede de acordo com o tipo (send or receive)
  * @param porta COM1, COM2, ...
  * @param flag TRANSMITTER (1) or RECEIVER (0)
  * @return 1 em caso de sucesso; 0 se houve algum erro na abertura da ligação
  */
int llopen(char* porta, int flag);

/**
  * escreve através da ligação de dados o conteúdo do array (buffer) de tamanho lenght
  * @param fd identificador da ligação de dados
  * @param buffer array de caracteres a transmitir
  * @param lenght comprimento do array de caracteres
  * @return número de caracteres escritos ou -1 em caso de erro
  */
int llwrite(int fd, unsigned char * buffer, int length);

/**
  * lê através da ligação de dados um array de caracteres e retorna-o
  * @param fd identificador da ligação de dados
  * @return array com os caracteres lidos ou -1 em caso de erro
  */
unsigned char * llread(int fd);

/**
  * termina a ligação de dados
  * @param fd identificador da ligação de dados
  * @param flag TRANSMITTER (1) or RECEIVER (0)
  * @return 0 em caso de sucesso e -1 em caso de erro
  */
int llclose(int fd, int flag);

#endif //FUNCTIONS_H
