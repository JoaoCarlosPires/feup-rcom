#include "functions.h"

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        fprintf(stderr, "usage: invalid number of parameters\n");
        exit(1);
    }

    char *url = argv[1]; 

    // ftp://<host>/<url-path> 
    // or
    // ftp://[<user>:<password>@]<host>/<url-path> 

    int index = 0;

    char *host = malloc(50);
    char *filepath = malloc(100);
    char *user = malloc(100);
    char *password = malloc(100);

    int state = FTP;
    int type = 0; // 0 - both provided, 1 - only user, 2 - only password, 3 - none
    int passwordFound = 0;
    int userFound = 0;

    for (long int i = 0; i < strlen(url); i++) {
        switch (state) {
            case FTP:
                if ((url[i] == '/') && (url[i + 1] == '/')) {
                    state = USER;
                    i++;
                }
                break;
            case USER:
                if (url[i] == '/') {
                    strcpy(host, user);
                    state = PATH;
                    index = 0;
                }
                else if (url[i] == ':') {
                    if (index != 0) userFound = 1;
                    state = PASSWORD;
                    index = 0;
                } else {
                    user[index] = url[i];
                    index++;
                }
                break;
            case PASSWORD:
                if (url[i] == '@') {
                    if (index != 0) passwordFound = 1;
                    state = HOST;
                    index = 0;
                } else {
                    password[index] = url[i];
                    index++;
                }
                break;
            case HOST:
                if (url[i] == '/') {
                    state = PATH;
                    index = 0;
                } else {
                    host[index] = url[i];
                    index++;
                }
                break;
            case PATH:
                filepath[index] = url[i];
                index++;
                break;
            default:
                break;
        }       
    }

    type = passwordFound == 1 && userFound == 1 ? 0 : passwordFound == 0 && userFound == 1 ? 1 : passwordFound == 1 && userFound == 0 ? 2 : 3;

    getHostInfo(host);

    establishConnection(inet_ntoa(*((struct in_addr *)h->h_addr)));

    char *messages[3];
    messages[2] = "pasv\n";

    if (type == 3) {        // case ftp://host/path
        

        // ask if wants anonymous mode
        write(STDOUT_FILENO, "Do you want to user anonymous mode? [y/N] ", 42);
        char choice[256];
        fgets(choice, sizeof(choice), stdin);
        char finalchoice;
        sscanf(choice, "%c", &finalchoice);

        if (finalchoice == 'y') {

            messages[0] = "user anonymous";
            messages[1] = "pass 123456";

        } else {
            // ask password and user
            
            write(STDOUT_FILENO, "Please provide an user: ", 24);
            char finalUser[256];
            fgets(finalUser, sizeof(finalUser), stdin);
            char *finalU = malloc(strlen("user ") + strlen(finalUser) + 1);
            strcpy(finalU, "user ");
            strcat(finalU, finalUser);
            messages[0] = finalU;

            write(STDOUT_FILENO, "Please provide a password: ", 27);
            char finalPassword[256];
            fgets(finalPassword, sizeof(finalPassword), stdin);
            char *finalP = malloc(strlen("pass ") + strlen(finalPassword) + 1);
            strcpy(finalP, "pass ");
            strcat(finalP, finalPassword);
            messages[1] = finalP;
        
        }

    } else if (type == 2) { // case ftp://:password@host/path
        
        char *finalPassword = malloc(strlen("pass ") + strlen(password) + 1);
        strcpy(finalPassword, "pass ");
        strcat(finalPassword, password);
        messages[1] = finalPassword;
        
        // ask user

        write(STDOUT_FILENO, "Please provide an user: ", 24);
        char finalUser[256];
        fgets(finalUser, sizeof(finalUser), stdin);
        char *finalU = malloc(strlen("user ") + strlen(finalUser) + 1);
        strcpy(finalU, "user ");
        strcat(finalU, finalUser);
        messages[0] = finalU;
    
    } else if (type == 1) { // case ftp://user:@host/path
    
        char *finalUser = malloc(strlen("user ") + strlen(user) + 1);
        strcpy(finalUser, "user ");
        strcat(finalUser, user);
        messages[0] = finalUser;

        // ask password

        write(STDOUT_FILENO, "Please provide a password: ", 27);
        char finalPassword[256];
        fgets(finalPassword, sizeof(finalPassword), stdin);
        char *finalP = malloc(strlen("pass ") + strlen(finalPassword) + 1);
        strcpy(finalP, "pass ");
        strcat(finalP, finalPassword);
        messages[1] = finalP;

    } else if (type == 0) { // case ftp://user:password@host/path
    
        char *finalUser = malloc(strlen("user ") + strlen(user) + 1);
        strcpy(finalUser, "user ");
        strcat(finalUser, user);
        messages[0] = finalUser;

        char *finalPassword = malloc(strlen("pass ") + strlen(password) + 1);
        strcpy(finalPassword, "pass ");
        strcat(finalPassword, password);
        messages[1] = finalPassword;
    }
    
    char buf = ' ';

    write(sockfd, messages[0], strlen(messages[0])); // user user
    write(STDOUT_FILENO, "> ", 2);
    write(STDOUT_FILENO, messages[0], strlen(messages[0]));
    write(sockfd, "\n", 1); // new line
    write(STDOUT_FILENO, "\n", 1);

    char * ip = malloc(50);

    int scnd_state = START;
    while (scnd_state != END)
    {

        read(sockfd, &buf, 1);
        write(STDOUT_FILENO, &buf, 1);

        switch (scnd_state)
        {
        case START:
            if (buf == '3')
            {
                read(sockfd, &buf, 1);
                write(STDOUT_FILENO, &buf, 1);
                if (buf == '3')
                {
                    read(sockfd, &buf, 1);
                    write(STDOUT_FILENO, &buf, 1);
                    if (buf == '1')
                    {
                        scnd_state = FIRST_ANS;
                    }
                }
            }
            break;
        case FIRST_ANS:
            if (buf == '\n')
            {
                write(sockfd, messages[1], strlen(messages[1])); // pass password
                write(STDOUT_FILENO, "> ", 2);
                write(STDOUT_FILENO, messages[1], strlen(messages[1]));
                write(sockfd, "\n", 1);
                write(STDOUT_FILENO, "\n", 1);
                scnd_state = SEARCH_SCND;
            }
            break;
        case SEARCH_SCND:
            if (buf == '2')
            {
                read(sockfd, &buf, 1);
                write(STDOUT_FILENO, &buf, 1);
                if (buf == '3')
                {
                    read(sockfd, &buf, 1);
                    write(STDOUT_FILENO, &buf, 1);
                    if (buf == '0')
                    {
                        scnd_state = SCND_ANS;
                    }
                }
            }
            break;
        case SCND_ANS:
            if (buf == '\n')
            {
                write(sockfd, messages[2], strlen(messages[2])); // pasv
                write(STDOUT_FILENO, "> ", 2);
                write(STDOUT_FILENO, messages[2], strlen(messages[2]));
                scnd_state = SEARCH_THIRD;
            }
            break;
        case SEARCH_THIRD:
            if (buf == '2')
            {
                read(sockfd, &buf, 1);
                write(STDOUT_FILENO, &buf, 1);
                if (buf == '2')
                {
                    read(sockfd, &buf, 1);
                    write(STDOUT_FILENO, &buf, 1);
                    if (buf == '7')
                    {
                        scnd_state = GETTING_LAST;
                    }
                }
            }
            break;
        case GETTING_LAST:
            if (buf == '(') {
                int i = 0;
                read(sockfd, &buf, 1);
                write(STDOUT_FILENO, &buf, 1);
                do {
                   ip[i] = buf;
                   i++;
                   read(sockfd, &buf, 1); 
                   write(STDOUT_FILENO, &buf, 1);
                } while (buf != ')');
                while(buf != '\n') {
                    read(sockfd, &buf, 1);    
                    write(STDOUT_FILENO, &buf, 1);
                }
                scnd_state = END;
            }
            break;
        default:
            break;
        }
    }

    int port = getServerPort(ip);

    establishConnection2(inet_ntoa(*((struct in_addr *)h->h_addr)), port);

    write(sockfd, "retr ", 5);
    write(STDOUT_FILENO, "> ", 2);
    write(STDOUT_FILENO, "retr ", 5);   
    write(sockfd, filepath, strlen(filepath));
    write(STDOUT_FILENO, filepath, strlen(filepath));
    write(sockfd, "\n", 1);    
    write(STDOUT_FILENO, "\n", 1);   

    char * filename = getFileName(filepath);
    int file = open(filename, O_CREAT | O_WRONLY, 0777);
	    
    char buffer[256];
    int bytes;
    while ((bytes = read(sockfd2, buffer, sizeof(256)))) {
        write(file, buffer, bytes);
    }   

  	close(file);

    close(sockfd);
    close(sockfd2);
    
    return 0;
}
