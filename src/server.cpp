#include <arpa/inet.h>
#include <cstdio>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

class UDP_Server{
private:
    int socket_fd;
    std::vector<struct sockaddr> client_list;

public:
    UDP_Server(char ip_addr[] , int port ){
        //this function creates a socket and binds the ip addr and the port on that socket
        //1.create a socket
        if((this->socket_fd = socket(AF_INET , SOCK_DGRAM , 0)) == -1)perror("socket file descriptor creation failed");

        //2. bind the configs to the socket
        struct sockaddr_in address ; 
        address.sin_port = htons(port); 
        address.sin_family = AF_INET ; //ipv4
         inet_pton(AF_INET, ip_addr, &(address.sin_addr));

        if(bind(this->socket_fd ,(sockaddr*)&address , sizeof(address)) ==-1){
            std::cout << ip_addr << std::endl ;
            perror("bind failed.");
            close(this->socket_fd);
            exit(1);
        }

    }
    
    void start(){
        int n ; 
        char buffer[1024] ;
        struct sockaddr client_addr ; // client socket buffer
        socklen_t addr_len = sizeof(client_addr);


        while(true){
            n = recvfrom(this->socket_fd, buffer, 100, 0 , &client_addr , &addr_len);
            if(this->client_list.size() == 0 ) this->client_list.push_back(client_addr);


            bool isSameAddr = 0;
            bool isSamePort = 0 ;
            bool isSameClient = 0;
            for(int i = 0 ; i<=this->client_list.size()-1; i++){
                isSameAddr = (((struct sockaddr_in*)&client_addr)->sin_addr.s_addr == (((struct sockaddr_in*)&this->client_list[i])->sin_addr.s_addr));
                isSamePort = (((struct sockaddr_in*)&client_addr)->sin_port == (((struct sockaddr_in*)&this->client_list[i])->sin_port));
                isSameClient = isSameAddr&& isSamePort ;
                if(isSameClient){
                    break;
                }
            }
            if(!isSameClient) this->client_list.push_back(client_addr);
            std::cout<<"captured:" << buffer << std::endl ; 
        
        //broadcast for all clients except for the sender
            for(int i =0 ; i<= this->client_list.size()-1 ; i++){
                isSameAddr = (((struct sockaddr_in*)&client_addr)->sin_addr.s_addr == (((struct sockaddr_in*)&this->client_list[i])->sin_addr.s_addr));
                isSamePort = (((struct sockaddr_in*)&client_addr)->sin_port == (((struct sockaddr_in*)&this->client_list[i])->sin_port));
                isSameClient = isSameAddr&& isSamePort ;
                if(!isSameClient && buffer[0]!='\0'){
                    sendto(this->socket_fd, buffer, 100, 0 ,(struct sockaddr* ) &this->client_list[i] , sizeof(this->client_list[i]));
                
                }
            }
        
        }   


    }





};





int main(){
    //socket file descriptor creation
    char ip[] = "127.0.0.1";
   UDP_Server server = UDP_Server( ip, 3000);
   server.start();
}