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

#define SERVER_PORT 21

#define h_addr2 h_addr_list[1]	The first address in h_addr_list. 

#define START 0
#define END 1
#define FIRST_ANS 2
#define SEARCH_SCND 3
#define SCND_ANS 4
#define SEARCH_THIRD 5

int	sockfd;
struct	sockaddr_in server_addr;
struct hostent *h;

void establishConnection(char * serverAddr){
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(serverAddr);	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);				/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    	}
	/*connect to the server*/
    	if(connect(sockfd, 
	           (struct sockaddr *)&server_addr, 
		   sizeof(server_addr)) < 0){
        	perror("connect()");
		exit(0);
	}

    printf("Connection established w/ success\n");

}

char * getFileName(char * ulr) {

}

void getHostInfo(char * hostName) {
	        
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

        if ((h=gethostbyname(hostName)) == NULL) {  
            herror("gethostbyname");
            exit(1);
        }
}

int main(int argc, char *argv[]) {

    if (argc != 2) {  
        fprintf(stderr,"usage: invalid number of parameters\n");
        exit(1);
    }

    char * url = argv[1]; //    ftp://[<user>:<password>@]<host>/<url-path>

    int hostRead = -1;
    char * host = malloc(50);
    for (long int i = 0; i < strlen(url) ; i++) {
        if (hostRead >= 0) {
            if (url[i] == '/') {
                hostRead = -1;
                break;
            }
            host[hostRead] = url[i];
            hostRead++;
        } else if ((url[i] == '/') && (url[i+1] == '/')) {
            hostRead = 0;
            i++;
        }
    }

    getHostInfo(host);

    establishConnection(inet_ntoa(*((struct in_addr *)h->h_addr)));

    
/*
< 331 Password required for euproprio.

< 230 User anonymouslogged in.

< 227 Entering Passive Mode (193,136,28,12,19,91)
*/

	char * messages[4] = {"user anonymous\n", "pass 123456\n", "pasv\n", "pub/parrot/last-sync.txt"};

    char buf = ' ';

    write(sockfd, messages[0], strlen(messages[0])); // user anonymous

    int state = START;
    while (state != END) {

        read(sockfd, &buf, 1);
        
        switch (state) {
            case START:
                if (buf == '3') {
                   read(sockfd, &buf, 1);
                   if (buf = '3') {
                       read(sockfd, &buf, 1);
                       if (buf = '1') {
                           state = FIRST_ANS;
                       }
                   } 
                }            
            break;
            case FIRST_ANS:
                if (buf == '\n') {
                    write(sockfd, messages[1], strlen(messages[1])); // pass qualquer-password
                    state = SEARCH_SCND;
                }
            break;
            case SEARCH_SCND:
                if (buf == '2') {
                   read(sockfd, &buf, 1);
                   if (buf = '3') {
                       read(sockfd, &buf, 1);
                       if (buf = '0') {
                           state = SCND_ANS;
                       }
                   } 
                } 
            break;
            case SCND_ANS:
                if (buf == '\n') {
                    write(sockfd, messages[2], strlen(messages[2])); // pasv
                    state = SEARCH_THIRD;
                }
            break; 
            case SEARCH_THIRD:
                if (buf == '2') {
                   read(sockfd, &buf, 1);
                   if (buf = '2') {
                       read(sockfd, &buf, 1);
                       if (buf = '7') {
                           state = END;
                       }
                   } 
                } 
            break;
        default:
            break;
        }
    }
    

    close(sockfd);
    return 0;
}
