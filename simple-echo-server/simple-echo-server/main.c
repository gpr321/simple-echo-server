//
//  main.c
//  simple-echo-server
//
//  Created by peironggao on 16/10/24.
//  Copyright © 2016年 gaopeirong. All rights reserved.
//

#include <stdio.h>

#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define ECHO_PORT                   (5556)
#define BUFFER_SIZE                 (1000)
#define LISTENQ                     (1024)

static inline int should_close(const char *str)
{
    int should_closed = 0;
    char *close_str = "close";
    int len = (int)strlen(close_str);
    for (int i = 0; i < len; i++)
    {
        if ((should_closed = (str[i] == close_str[i])) == 0)
        {
            break;
        }
    }
    
    return should_closed;
}

static inline ssize_t readline(int sock_fd, void *ptr, size_t maxlen)
{
    ssize_t n , rc;
    char c , *buffer;
    
    buffer = ptr;
    for ( n = 1; n < maxlen; n++)
    {
        if ((rc = read(sock_fd, &c, 1)) == 1)
        {
            *buffer++ = c;
            if (c == '\n')
            {
                *buffer = '\0';
                break;
            }
        }
        else if (rc == 0)
        {
            if (n == 1)
                return 0;
            else
                break;
        }
        else
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
    }
    
    return n;
}

static inline ssize_t writeline(int sock_fd, void *ptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;
    
    buffer = ptr;
    nleft  = n;
    
    while (nleft > 0)
    {
        if ((nwritten = write(sock_fd, buffer, nleft)) <= 0)
        {
            if ( errno == EINTR )
                nwritten = 0;
            else
                return -1;
        }
        
        nleft -= nwritten;
        buffer += nwritten;
    }
    
    return n;
}

int main(int argc, const char * argv[])
{
    int fd_socket = 0;
    int conn_s = 0;
    
    int port = ECHO_PORT;
    struct sockaddr_in servaddr;
    char buffer[BUFFER_SIZE];
    
    if ( argc == 2 )
    {
        char *endptr;
        port = (int)strtol(argv[1], &endptr, 0);
    }
    
    if ( (fd_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
    {
        fprintf(stderr, "ECHOSERV: Error creating listening socket.\n");
        exit(EXIT_FAILURE);
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    
    if (bind(fd_socket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        fprintf(stderr, "ECHOSERV: Error calling bind()\n");
        exit(EXIT_FAILURE);
    }
    
    if (listen(fd_socket, LISTENQ) < 0)
    {
        fprintf(stderr, "ECHOSERV: Error calling listen()\n");
        exit(EXIT_FAILURE);
    }
    printf("server started port = %d \n", port);
    if ((conn_s = accept(fd_socket, NULL, NULL)) < 0)
    {
        fprintf(stderr, "ECHOSERV: Error calling accept()\n");
        exit(EXIT_FAILURE);
    }
    printf("a client connected \n");
    
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        if (readline(conn_s, buffer, BUFFER_SIZE) > 0)
        {
            int len = (int)strlen(buffer);
            printf("read str from client %s\n", buffer);
            writeline(conn_s, buffer, len);
        }
        
        if (should_close(buffer))
            break;
        else
            continue;
    }
    
    if (close(conn_s) < 0)
    {
        fprintf(stderr, "ECHOSERV: Error calling close()\n");
        exit(EXIT_FAILURE);
    }
    printf("server closed \n");
    return 0;
}


