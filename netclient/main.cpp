#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFSIZE 128

int main(int argc, char *argv[]){
    int sock, str_len, status;
    char buf[BUFSIZE];

    struct sockaddr_in server_addr;

    if(argc !=3 ){
        printf("usage: ./netclient <ip> <port>\n");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    if((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Can't open stream socket\n");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    //server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(atoi(argv[2]));
    //server_addr.sin_port = htons(1234);

    if((connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))) == -1){
        printf("Connect Error\n");
        return -1;
    }
    int pid = fork();
    if(pid > 0){
        while(1){
            if(waitpid(pid, &status, WNOHANG))
                return 0;
            printf("msg: ");
            gets(buf);
            write(sock, buf, strlen(buf)+1);
        }
    }

    else if(pid == 0){
        while(1){
            str_len = read(sock, buf, BUFSIZE);
            if(str_len == 0){
                close(sock);
                return 0;
            }
            printf("server: %s\n", buf);
        }
    }

    return 0;
}
