#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define SERVER_PORT 21

#define h_addr2 h_addr_list[1] The first address in h_addr_list.

#define START 0
#define END 1
#define FIRST_ANS 2
#define SEARCH_SCND 3
#define SCND_ANS 4
#define SEARCH_THIRD 5
#define GETTING_LAST 6

#define FTP 0
#define USER 1
#define PASSWORD 2
#define HOST 3
#define PATH 4

int sockfd;
struct sockaddr_in server_addr;

int sockfd2;
struct sockaddr_in server_addr2;

struct hostent *h;

void establishConnection(char *serverAddr);
void establishConnection2(char *serverAddr, int port);
char *getFileName(char *filepath);
void getHostInfo(char *hostName);
int getServerPort(char * arr);

#endif //FUNCTIONS_H