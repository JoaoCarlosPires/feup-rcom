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

#define C_RR 0b10000101
#define FLAG 0b01111110
#define A1 0b00000011
#define A2 0b00000001
#define C_SET 0b00000011
#define C_UA 0b00000111
#define C_DISC 0b00001011
#define C_RR 0b10000101

int STOP;
int UA_RCV;
int allarms_called;
int alarm_active;

void alarmHandler();

unsigned char * bcc_cal(unsigned char * buffer);

int stateMachine(int curr_state, unsigned char *input, int C, int A);

unsigned char * stuffing(unsigned char * byteArray, int * len);

unsigned char * destuffing(unsigned char * byteArray, int * len);

#endif //UTILS_H