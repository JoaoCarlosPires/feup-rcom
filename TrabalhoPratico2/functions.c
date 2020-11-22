#include "functions.h"

void establishConnection(char *serverAddr)
{

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(serverAddr); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT);           /*server TCP port must be network byte ordered */

    /*open an TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(0);
    }
    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(0);
    }

    printf("Connection established w/ success\n");
}

void establishConnection2(char *serverAddr, int port)
{

    /*server address handling*/
    bzero((char *)&server_addr2, sizeof(server_addr2));
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_addr.s_addr = inet_addr(serverAddr); /*32 bit Internet address network byte ordered*/
    server_addr2.sin_port = htons(port);           /*server TCP port must be network byte ordered */

    /*open an TCP socket*/
    if ((sockfd2 = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(0);
    }
    /*connect to the server*/
    if (connect(sockfd2,
                (struct sockaddr *)&server_addr2,
                sizeof(server_addr2)) < 0)
    {
        perror("connect()");
        exit(0);
    }

    printf("2nd Connection established w/ success\n");
}

char *getFileName(char *filepath) {
    int lastBarIndex = 0;
    for (int i = 0; i < strlen(filepath); i++) {
        if (filepath[i] == '/') {
            lastBarIndex = i+1;
        }
    }

    char * filename = malloc(50);
    int k = 0;
    for (int i = lastBarIndex; i < strlen(filepath); i++, k++) {
        filename[k] = filepath[i];
    }

    return filename;
}


/*
struct hostent {
	char    *h_name;	    Official name of the host. 
    char    **h_aliases;	A NULL-terminated array of alternate names for the host. 
	int     h_addrtype;	    The type of address being returned; usually AF_INET.
    int     h_length;	    The length of the address in bytes.
	char    **h_addr_list;	A zero-terminated array of network addresses for the host. 
		                    Host addresses are in Network Byte Order. 
};

#define h_addr h_addr_list[0]	The first address in h_addr_list. 
*/
void getHostInfo(char *hostName)
{

    if ((h = gethostbyname(hostName)) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }
}

int getServerPort(char * arr) {
    int i = 0;
    char *p = strtok (arr, ",");
    char *array[6];

    while (p != NULL)
    {
        array[i++] = p;
        p = strtok (NULL, ",");
    }

    int port = 256*atoi(array[4]) + atoi(array[5]);
    return port;
}
