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
#define FALSE 0
#define TRUE 1

#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4

#define FLAG 0b01111110
#define A 0b00000011
#define C 0b00000011

#define TIMEOUT 3

int allarms_called;
unsigned char TRAMA_SET[5];
struct termios oldtio, newtio;
int UA_RCV;
int alarm_active;
volatile int STOP;

unsigned char bcc_cal(char * buffer);

/**
  * inicia a comunicação e procede de acordo com o tipo (send or receive)
  * @param porta COM1, COM2, ...
  * @param flag TRANSMITTER (1) or RECEIVER (0)
  * @return identificador da ligação de dados ou -1 em caso de erro
  */
int llopen(char* porta, int flag);

/**
  * escreve através da ligação de dados o conteúdo do array (buffer) de tamanho lenght
  * @param fd identificador da ligação de dados
  * @param buffer array de caracteres a transmitir
  * @param lenght comprimento do array de caracteres
  * @return número de caracteres escritos ou -1 em caso de erro
  */
int llwrite(int fd, char * buffer, int length);

/**
  * lê através da ligação de dados um conjunto de caracteres (cujo tamanho retorna) e guarda-os em buffer
  * @param fd identificador da ligação de dados
  * @param buffer array de caracteres recebidos
  * @return comprimento do array (número de caracteres lidos) ou -1 em caso de erro
  */
int llread(int fd, char * buffer);

/**
  * termina a ligação de dados
  * @param fd identificador da ligação de dados
  * @return 0 em caso de sucesso e -1 em caso de erro
  */
int llclose(int fd);

void stateMachine(int *curr_state, unsigned char *input);

#endif //FUNCTIONS_H
