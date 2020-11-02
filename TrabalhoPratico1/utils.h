#ifndef UTILS_H
#define UTILS_H

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

#define FALSE 0
#define TRUE 1

#define START 0
#define FLAG_RCV 1
#define A_RCV 2
#define C_RCV 3
#define BCC_RCV 4
#define FINISH 5
#define DATA 6
#define BCC2_RCV 7

#define MAXALARMS 3

#define C_RR 0b10000101
#define FLAG 0b01111110
#define A1 0b00000011
#define A2 0b00000001
#define C_SET 0b00000011
#define C_UA 0b00000111
#define C_DISC 0b00001011
#define C_RR 0b10000101

#define C_Data 1
#define C_Begin 2
#define C_End 3
#define T_FileSize 0
#define T_FileName 1

#define STUFFING 0
#define NOSTUFFING 1

#define ESCAPE 0b01111101

int STOP;
int UA_RCV;
int allarms_called;
int alarm_active;

/**
 * alarmHandler - incrementa a variável allarms_called e coloca a variável
 * alarm_active a TRUE
*/
void alarmHandler();

/**
 * bcc_cal - calcula o bcc2 com ou sem stuffing (depende do terceiro argumento)
 * @param buffer array de caracteres sobre o qual o bcc2 será calculado
 * @param lenght comprimento do array de caracteres
 * @param stuff_flag STUFFING (bcc2 com stuffing) ou DESTUFFING (bcc2 sem stuffing) 
 * @return array de caracteres com o valor de bcc2 (c/ stuffing - array c/ 1 ou 2 elem.; s/ stuffing - array c/ 1 elem.)
*/
unsigned char * bcc_cal(unsigned char * buffer,int length, int stuff_flag);

/**
 * stateMachine - máquina de estados que recebe o estado atual e verifica se o input é o próximo estando,
 * agindo conforme este último resultado.
 * @param curr_state Estado atual
 * @param input Valor recebido
 * @param C Valor do C para a ocasião
 * @param A Valor do A para a ocasião
 * @return Estado atual em caso de sucesso; valor negativo em caso de erro
*/
int stateMachine(int curr_state, unsigned char *input, int C, int A);

/**
 * stuffing - recebe um array de caracteres e o seu comprimento e efetua o mecanismo de stuffing sobre o mesmo, retornando-o
 * @param byteArray array de caracteres a percorrer para fazer o stuffing
 * @param len tamanho do array de caracteres
 * @return array de caracteres com stuffing aplicado
*/
unsigned char * stuffing(unsigned char * byteArray, int * len);

/**
 * destuffing - recebe um array de caracteres e o seu comprimento e efetua o mecanismo de destuffing sobre o mesmo, retornando-o
 * @param byteArray array de caracteres a percorrer para fazer o destuffing
 * @param len tamanho do array de caracteres
 * @return array de caracteres com destuffing aplicado
*/
unsigned char * destuffing(unsigned char * byteArray, int * len);

#endif //UTILS_H