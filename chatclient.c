/*******************************************************************************
* Name        : chatclient.c
* Author      : Avaneesh Kolluri worked with Akhilesh Reddy.
* Date        : 4/17/20
* Description : hw 7
* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
******************************************************************************/

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <limits.h>
#include <sys/select.h>
#include "util.h"

int client_socket = -1;
char username[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin() {
   // printf("IM in stdin helper\n");
    int retval;
    if ((retval = get_string(outbuf,MAX_MSG_LEN)) == TOO_LONG){
        //printf("NUM CHARS %ld  :: %s \n",strlen(outbuf),outbuf);
        fprintf(stderr,"Sorry, limit your message to %d characters.\n" , MAX_MSG_LEN);
    } else if(retval == NO_INPUT){
        return EXIT_SUCCESS;
    }
    else if(send(client_socket, outbuf, strlen(outbuf), 0) < 0){
        fprintf(stderr, "Error: Failed to send message to server. %s.\n", strerror(errno));
        //return EXIT_FAILURE;
    }

    if (strcmp(outbuf,"bye") == 0){
        printf("Goodbye.\n");
        close(client_socket);
        exit(EXIT_SUCCESS);
    }
    
    return EXIT_SUCCESS;
}

int handle_client_socket() {
   // printf("IM in client helper\n");
    int bytes_recvd;
    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0)) == -1) {
        fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n", strerror(errno));
    }
    
    if(bytes_recvd == 0){
        fprintf(stderr, "\nConnection to server has been lost.\n");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    inbuf[bytes_recvd] = '\0';
    
    if(strcmp("bye", inbuf) == 0){
        printf("\nServer initiated shutdown.\n");
    } else{
        printf("\n%s\n",inbuf);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    
    struct sockaddr_in serv_addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    memset(&serv_addr, 0, addrlen);
    int ip_conversion = inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);
    if(ip_conversion == 0){
        fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    } else if (ip_conversion < 0) {
        fprintf(stderr, "Error: Failed to convert IP address. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    
    int port_no;
    char usage[4096] = "port number";
    if(parse_int(argv[2], &port_no, usage) == false){
        return EXIT_FAILURE;
    }
    
    
    
    if(port_no < 1024 || port_no>65535){
        fprintf(stderr, "Error: Port must be in range [1024, 65535].\n");
        return EXIT_FAILURE;
    }
    int userval;
    bool stay = true;
    
    while(stay){
        printf("Enter your username: ");
        fflush(stdout);
        if ((userval = get_string(username,MAX_NAME_LEN+1)) == TOO_LONG){
            //printf("NUM CHARS %ld  :: %s \n",strlen(outbuf),outbuf);
            fprintf(stderr,"Sorry, limit your username to %d characters.\n" , MAX_NAME_LEN);
        } else if(userval == NO_INPUT){
            continue;
        } else{
            break;
        }
        //username[strlen(username) -1] = '\0';
    }
    int retval = EXIT_SUCCESS;

    int bytes_recvd;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_no);
    printf("Hello, %s. Let's try to connect to the server.\n", username);
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: Failed to create socket. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }
    
    if(connect(client_socket,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0){
        fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }
    
    if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN, 0)) == -1) {
        fprintf(stderr, "Error: Failed to receive message from server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }
    inbuf[bytes_recvd] = '\0';
    
    if(bytes_recvd == 0){
        fprintf(stderr, "All connections are busy. Try again later.\n");
        retval = EXIT_FAILURE;
        goto EXIT;
    }
    
    printf("\n");
    strcpy(outbuf,inbuf);
    printf("%s", outbuf);
    printf("\n");
    printf("\n");
    
    if(send(client_socket, username, strlen(username), 0) < 0){
        fprintf(stderr, "Error: Failed to send username to server. %s.\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto EXIT;
    }
    

    fd_set cfds;
    
    //struct timeval tv;

    //while loop
    while(true){
        memset(outbuf, 0, sizeof outbuf);
        printf("[%s]: ", username);
        fflush(stdout);
//        if(fgets(outbuf,MAX_MSG_LEN+1,stdin) == NULL){
//            fprintf(stderr, "fgets failed.\n");
//            return EXIT_FAILURE;
//        }
        //printf("outbuf: %s \n", outbuf);
       

        int returnval;
        
        FD_ZERO(&cfds);
        FD_SET(STDIN_FILENO, &cfds);
        FD_SET(client_socket, &cfds);
//        int maxval = STDIN_FILENO;
//        if(maxval < rfds){
//            maxval = rfds;
//        }
        //printf("IMHERE 219\n");
        returnval = select(client_socket+1, &cfds, NULL, NULL, NULL);
        
        if(returnval == -1){
            fprintf(stderr, "Select Failed.\n");
            goto EXIT;
        } else if(returnval){
            if((FD_ISSET(STDIN_FILENO, &cfds))){
                //printf("      1          HIII  \n");
                handle_stdin(); 
            
            }
            if((FD_ISSET(client_socket, &cfds))){
                if(handle_client_socket() == -1){
                    fprintf(stderr, "Handle Client Failed\n");
                    retval = EXIT_FAILURE;
                    goto EXIT;
                }
            }
            

        }
    
        //printf("EOL\n");
    }
    EXIT:
        if(fcntl(client_socket, F_GETFD) >= 0) {
            close(client_socket);
        }
        return retval;
    
    
    
}

