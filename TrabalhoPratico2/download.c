#include "functions.h"

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: invalid number of parameters\n");
        exit(1);
    }

    char *url = argv[1]; //    ftp://[<user>:<password>@]<host>/<url-path>

    int hostRead = -1;
    int j = 0;
    char *host = malloc(50);
    char *filepath = malloc(100);
    for (long int i = 0; i < strlen(url); i++)
    {
        if (hostRead == -2) {
            filepath[j] = url[i];
            j++;
        }
        else if (hostRead >= 0)
        {
            if (url[i] == '/')
            {
                hostRead = -2;
            } else {
            host[hostRead] = url[i];
            hostRead++;
            }
        }
        else if ((url[i] == '/') && (url[i + 1] == '/'))
        {
            hostRead = 0;
            i++;
        }
    }

    getHostInfo(host);

    establishConnection(inet_ntoa(*((struct in_addr *)h->h_addr)));

    char *messages[4] = {"user anonymous\n", "pass 123456\n", "pasv\n", "pub/parrot/last-sync.txt"};

    char buf = ' ';

    write(sockfd, messages[0], strlen(messages[0])); // user anonymous

    char * ip = malloc(50);

    int state = START;
    while (state != END)
    {

        read(sockfd, &buf, 1);

        switch (state)
        {
        case START:
            if (buf == '3')
            {
                read(sockfd, &buf, 1);
                if (buf = '3')
                {
                    read(sockfd, &buf, 1);
                    if (buf = '1')
                    {
                        state = FIRST_ANS;
                    }
                }
            }
            break;
        case FIRST_ANS:
            if (buf == '\n')
            {
                write(sockfd, messages[1], strlen(messages[1])); // pass qualquer-password
                state = SEARCH_SCND;
            }
            break;
        case SEARCH_SCND:
            if (buf == '2')
            {
                read(sockfd, &buf, 1);
                if (buf = '3')
                {
                    read(sockfd, &buf, 1);
                    if (buf = '0')
                    {
                        state = SCND_ANS;
                    }
                }
            }
            break;
        case SCND_ANS:
            if (buf == '\n')
            {
                write(sockfd, messages[2], strlen(messages[2])); // pasv
                state = SEARCH_THIRD;
            }
            break;
        case SEARCH_THIRD:
            if (buf == '2')
            {
                read(sockfd, &buf, 1);
                if (buf = '2')
                {
                    read(sockfd, &buf, 1);
                    if (buf = '7')
                    {
                        state = GETTING_LAST;
                    }
                }
            }
            break;
        case GETTING_LAST:
            if (buf == '(') {
                int i = 0;
                read(sockfd, &buf, 1);
                do {
                   ip[i] = buf;
                   i++;
                   read(sockfd, &buf, 1); 
                } while (buf != ')');
                while(buf != '\n') {
                    read(sockfd, &buf, 1);    
                }
                state = END;
            }
            break;
        default:
            break;
        }
    }

    int port = getServerPort(ip);

    establishConnection2(inet_ntoa(*((struct in_addr *)h->h_addr)), port);

    write(sockfd, "retr ", 5);  
    write(sockfd, filepath, strlen(filepath));
    write(sockfd, "\n", 1);     

    char * contents = malloc(1080);
    int k = 0;
    while (read(sockfd2, &contents[k], 1) != 0) {
        k++;
    }

    char * filename = getFileName(filepath);
	FILE *file = fopen(filename, "wb+");

    fwrite(contents, 1, strlen(contents), file);

  	fclose(file);

    close(sockfd);
    close(sockfd2);
    return 0;
}
