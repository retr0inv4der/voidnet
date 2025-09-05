#include <arpa/inet.h>
#include <cstdio>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

int main(){
    //socket file descriptor creation
    int serverfd ;
    if((serverfd = socket(AF_INET , SOCK_DGRAM , 0)) == -1)perror("socket file descriptor creation failed");

    //define the server ip and the port number
    struct sockaddr_in address ; 
    address.sin_port = htons(3000); //port 3000
    address.sin_family = AF_INET ; //ipv4
    address.sin_addr.s_addr = INADDR_ANY; //listen on all interfaces

    //bind the socket
    if(bind(serverfd ,(sockaddr*)&address , sizeof(address)) ==-1){
        perror("bind failed.");
        close(serverfd);
        exit(1);
    }
    std::cout << "listening ..." << std::endl ; 

    //start getting msgs
    int n ; 
    char buffer[1024] ;
    struct sockaddr client_addr ; // client socket buffer
    socklen_t addr_len = sizeof(client_addr);

    std::vector<struct sockaddr> client_list ;

    while(true){
        n = recvfrom(serverfd, buffer, 100, 0 , &client_addr , &addr_len);
        if(client_list.size() == 0 ) client_list.push_back(client_addr);

        bool exist = 0;
        for(int i = 0 ; i<=client_list.size() ; i++){
            if((((struct sockaddr_in*)&client_addr)->sin_addr.s_addr == (((struct sockaddr_in*)&client_list[i])->sin_addr.s_addr)) && (((struct sockaddr_in*)&client_addr)->sin_port == (((struct sockaddr_in*)&client_list[i])->sin_port))){
                exist =1 ;
                break; 
            }
        }
        if(!exist) client_list.push_back(client_addr);
        std::cout<<"captured:" << buffer << std::endl ; 

        //broadcast for all clients
        for(int i =0 ; i<= client_list.size()-1 ; i++){
            sendto(serverfd, buffer, 100, 0 ,(struct sockaddr* ) &client_list[i] , sizeof(client_list[i]));

        }
    }   
}