#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

int main(){
    //create the socket 
    int clientfd ; 
    if((clientfd = socket(AF_INET , SOCK_DGRAM , 0)) == -1){
        perror("failed in socket creation");
        exit(1) ; 
    }

    //enable the broadcast mode 
    int broadcastmode = 1 ; 
        if (setsockopt(clientfd, SOL_SOCKET, SO_BROADCAST,
                   &broadcastmode, sizeof(broadcastmode)) < 0) {
        perror("setsockopt failed");
        close(clientfd);
        exit(1);
    }

    //init the dest addr
    struct sockaddr_in dest_addr; 
    dest_addr.sin_family = AF_INET ;
    dest_addr.sin_port = htons(3000) ;
    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    socklen_t dest_len = sizeof(dest_addr) ;
    char message[100] ; 
    //main loop
    while (true) {
        std::cout << "enter a message :" ; 
        std::cin.getline(message , sizeof(message));
        int n =  sendto(clientfd, message, sizeof(message), 0, (struct sockaddr*)&dest_addr ,dest_len );
        if (n < 0) {
            perror("sendto failed");
        }
    }
}