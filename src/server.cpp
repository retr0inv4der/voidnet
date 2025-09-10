#include <arpa/inet.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

class UDP_Server{
private:
    int socket_fd;
    std::vector<struct sockaddr> client_list;

enum PacketType {
    MESSAGE = 1,
    ACK = 2
};

struct MessagePacket {
    uint32_t type;   // MESSAGE
    uint32_t seq;
    uint32_t size;
    char data[256];
};

struct AckPacket {
    uint32_t type;   // ACK
    uint32_t seq;
};


struct FullPacket{
    struct MessagePacket Message;
    struct sockaddr_in addr;
};


std::vector<FullPacket> ReceivedPackets;



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

    void PacketReceiver(){
        int n ; 
        char buffer[sizeof(MessagePacket)];
        struct sockaddr client_addr ; 
        socklen_t addr_len = sizeof(client_addr); 
        struct FullPacket final_pkt_buff;
        
        while(true){
            n = recvfrom(this->socket_fd, buffer, sizeof(buffer), 0 , &client_addr, &addr_len); 
            if(this->client_list.size() == 0 ) this->client_list.push_back(client_addr);
            //add the packet into the ReceivedPackets list
            
            memcpy(&(final_pkt_buff.Message), buffer, sizeof(buffer));
            memcpy(&(final_pkt_buff.addr), &client_addr, sizeof(client_addr));
            this->ReceivedPackets.push_back(final_pkt_buff);
            //imidiatly send ACK
            struct AckPacket ack;
            ack.type = ACK;
            ack.seq = final_pkt_buff.Message.seq ;
            sendto(this->socket_fd, &ack, sizeof(ack), 0, &client_addr, addr_len);

            //add the client into the client list
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
        }
    }

    void RegisterReceiver(){
        std::thread listener(&UDP_Server::PacketReceiver , this);
        listener.detach();
    }

    struct AckPacket WaitForAck(struct sockaddr* client_addr , uint32_t seq){

    }

    void broadcast(){
        //this function should broadcast every received packet to all clients except the sender 
        bool isSameAddr = 0;
        bool isSamePort = 0 ;
        bool isSameClient = 0;
        size_t queue_size = this->ReceivedPackets.size();
        size_t client_list_size = this->client_list.size();
        socklen_t addrSize ;

        for(int i =0 ; i<queue_size ; i++){ //iterate the msgs
            for(int j = 0 ; j<client_list_size ; j++){ //iterate each client
                //check if the pkt source addr is the same as the one in the client list and the send it to the client if not
                isSameAddr = (this->ReceivedPackets[i].addr.sin_addr.s_addr) == (((struct sockaddr_in*)&this->client_list[j])->sin_addr.s_addr);
                isSamePort = (this->ReceivedPackets[i].addr.sin_port) == (((struct sockaddr_in*)&this->client_list[j])->sin_port);
                isSameClient = isSameAddr && isSamePort ;
                if(!isSameClient){ //if they are not the same clients then send the packet to it
                    addrSize = sizeof(this->client_list[j]);
                    sendto(this->socket_fd, &(this->ReceivedPackets[i].Message), sizeof(MessagePacket), 0, &(this->client_list[j]),addrSize );
                    //TODO : IMPLEMENT THE ACK SYSTEM

                } 
            }
            
                
        }
    }

    void start(){
        this->RegisterReceiver();
        while(true){
        //broadcast for all clients except for the sender
            this->broadcast();
        }   

    }

};





int main(){
    //socket file descriptor creation
    char ip[] = "127.0.0.1";
   UDP_Server server = UDP_Server( ip, 3000);
   server.start();
}