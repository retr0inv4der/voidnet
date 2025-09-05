#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <iostream>

void receiveMessages(int sock){
    char buffer[1024];
    sockaddr_in fromAdrr;
    socklen_t fromLen = sizeof(fromAdrr);
    while(true){
        int bytesReceived = recvfrom(sock, buffer, sizeof(buffer), 0 , (struct sockaddr*)&fromAdrr , &fromLen );
        if(bytesReceived>0){
            buffer[bytesReceived] ='\0';
            std::cout<<"\n[SERVER]:" << buffer << std::endl <<" > " ; 
            std::cout.flush();

        }
    }
}


int main(){
    //create the socket 
    int clientfd ; 
    if((clientfd = socket(AF_INET , SOCK_DGRAM , 0)) == -1){
        perror("failed in socket creation");
        exit(1) ; 
    }



    //init the dest addr
    struct sockaddr_in dest_addr; 
    dest_addr.sin_family = AF_INET ;
    dest_addr.sin_port = htons(3000) ;
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    socklen_t dest_len = sizeof(dest_addr) ;
    char message[100] ; 
    //main loop

    //register the receiver as well
    std::thread listener(receiveMessages , clientfd); 
    while (true) {
        std::cout << " > " ; 
        std::cin.getline(message , sizeof(message));
        int n =  sendto(clientfd, message, sizeof(message), 0, (struct sockaddr*)&dest_addr ,dest_len );
        if (n < 0) {
            perror("sendto failed");
        }
    }
}