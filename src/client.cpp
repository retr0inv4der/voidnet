#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <iostream>


class UDP_Client {
private:
    int sockfd ;
    struct sockaddr_in dest_addr;
    socklen_t dest_len = sizeof(this->dest_addr) ;

public:
    UDP_Client(char ip_addr[] , int port){
        //this function inits the socketfd and then sets the dest address 
        //1. init the  socket 
        if((this->sockfd = socket(AF_INET , SOCK_DGRAM , 0)) == -1){
            perror("failed in socket creation");
            exit(1) ; 
        }

        //2. set the dest address
        this->dest_addr.sin_family = AF_INET ;
        this->dest_addr.sin_port = htons(port) ;
        this->dest_addr.sin_addr.s_addr = inet_addr(ip_addr);
        
    }

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

    void RegisterReveiver(){
        std::thread listener(&UDP_Client::receiveMessages ,this ,  this->sockfd);
        listener.detach();
    }

    void start(){
            this->RegisterReveiver();

            char message[1024];
            while (true) {
            std::cout << " > " ; 
            std::cin.getline(message , sizeof(message));
            int n =  sendto(this->sockfd, message, sizeof(message), 0, (struct sockaddr*)&(this->dest_addr) ,this->dest_len );
            if (n < 0) {
                perror("sendto failed");
            }
        }
    }
};




int main(){
    char ip[] = "127.0.0.1";
    UDP_Client client  = UDP_Client(ip , 3000);
    client.start();
}