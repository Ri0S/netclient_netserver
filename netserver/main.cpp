#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <list>

#define BUFSIZE 128
#define EPOLL_SIZE 20

using namespace std;

class Client{
private:
    int sock;
    struct sockaddr_in client_addr;
public:
    int getsock(){
        return sock;
    }

    Client(int server_sock){
        memset(&client_addr, 0, sizeof(client_addr));
        int len = sizeof(client_addr);
        if((sock = accept(server_sock, (struct sockaddr*)&client_addr, (socklen_t *)&len)) == -1){
            printf("Accept Failed.\n");
        }
        else
            printf("New Client %d Connect", sock);
        printf(" \n");
    }

    int echomsg(string str){
        write(sock, str.c_str(), str.length()+1);
    }
};

int main(int argc, char *argv[])
{
    int eb = 0;
    struct sockaddr_in server_addr;
    char buf[BUFSIZE];
    int server_sock;
    int str_len;

    list<Client> clientlist;

    struct epoll_event *ep_events;
    struct epoll_event event;
    int epfd, event_cnt;
    int port = 1234;

    if(argc < 2){
        printf("usage: ./netserver <port> <opt>\n");
        return 0;
    }
    if(argc >= 3){
        if(!strcmp("-eb", argv[2]))
            eb = 1;
    }

    port = atoi(argv[1]);

    if((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Can't open stream socket\n");
        return -1;
    }
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if((bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr))) == -1){
        printf("Can't bind.\n");
        return -1;
    }

    if((listen(server_sock, 5)) < 0){
        printf("Can't listen connection.\n");
        return -1;
    }

    epfd=epoll_create(EPOLL_SIZE);
    ep_events=(struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

    event.events = EPOLLIN;
    event.data.fd = server_sock;
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock, &event);

    while(1){
        event_cnt = epoll_wait(epfd, ep_events, EPOLL_SIZE, -1);
        if(event_cnt == -1){
            puts("epoll_wait() error");
            break;
        }

        for(int i=0; i<event_cnt; i++){
            if(ep_events[i].data.fd == server_sock){
                Client clnt(server_sock);
                if(clnt.getsock() != -1){
                    event.events = EPOLLIN;
                    event.data.fd = clnt.getsock();
                    epoll_ctl(epfd, EPOLL_CTL_ADD, clnt.getsock(), &event);
                    clientlist.push_back(clnt);
                }
            }
            else{
                str_len = read(ep_events[i].data.fd, buf, BUFSIZE);
                if(!strcmp("quit", buf) || (str_len == 0)){
                    int tfd;
                    for(list<Client>::iterator iter = clientlist.begin(); iter != clientlist.end(); iter++){
                        if(iter->getsock() == ep_events[i].data.fd){
                            tfd = iter->getsock();
                            iter = clientlist.erase(iter);
                            close(tfd);
                            printf("Client %d Disconnected\n", tfd);
                        }
                    }
                }
                else{
                    printf("%d: %s\n", ep_events[i].data.fd, buf);
                    if(eb){
                        for(list<Client>::iterator iter = clientlist.begin(); iter != clientlist.end(); iter++)
                            iter->echomsg(buf);
                    }
                    else
                        write(ep_events[i].data.fd, buf, str_len);
                }
            }
        }
    }
}
