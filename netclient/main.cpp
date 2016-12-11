#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 128

int main(int argc, char *argv[]){
    int sock, str_len;
    char buf[BUFSIZE];

    struct sockaddr_in server_addr;

    if(argc !=3 ){
        printf("usage: ./netclient <ip> <port>");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    if((connect(sock, (struct sockaddr *) & server_addr, sizeof(server_addr))) < 0){
        printf("Connect Error\n");
        return -1;
    }

    gets(buf);
    write(sock, buf, strlen(buf)+1);
    while((str_len = read(sock, buf, BUFSIZE)) > 0){
        printf("%s\n", buf);
        gets(buf);
        write(sock, buf, strlen(buf)+1);
    }

    return 0;
}
