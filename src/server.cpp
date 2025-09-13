#include <arpa/inet.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <ostream>
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

struct FullAck{
    struct AckPacket Ack ; 
    struct sockaddr_in addr ; 
};
std::vector<FullPacket> ReceivedPackets;

std::vector<FullAck> ReceivedAcks; 

    std::mutex mtx ; 

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

        // struct timeval tv; 
        // tv.tv_sec =2 ; 
        // tv.tv_usec = 0 ;
        // setsockopt(this->socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv , sizeof(tv));

    }

    void PacketReceiver(){
        int n ; 
        char buffer[sizeof(MessagePacket)];
        struct sockaddr client_addr ; 
        socklen_t addr_len = sizeof(client_addr); 
        struct FullPacket final_pkt_buff;
        struct FullAck final_ack;
        uint32_t type;
        this->mtx.lock();
        int socket = this->socket_fd ;
        this->mtx.unlock();
        while(true){
             
            
            n = recvfrom(socket, buffer, sizeof(buffer), 0 , &client_addr, &addr_len); 

            this->mtx.lock();
            if(this->client_list.size() == 0 ) this->client_list.push_back(client_addr);
            //add the packet into the ReceivedPackets list
            //decide what the type of the packet is 
            memcpy(&type, buffer, sizeof(type));
           
            if(type == ACK){
                memcpy(&(final_ack.Ack), buffer, sizeof(AckPacket));
                memcpy(&(final_ack.addr), &client_addr, sizeof(client_addr));
                
                this->ReceivedAcks.push_back(final_ack);
                std::cout <<"size of ack list :" << this->ReceivedAcks.size() <<std::endl;

            }else if(type  == MESSAGE){
                memcpy(&(final_pkt_buff.Message), buffer, sizeof(buffer));
                memcpy(&(final_pkt_buff.addr), &client_addr, sizeof(client_addr));
                this->ReceivedPackets.push_back(final_pkt_buff);
                std::cout <<"captured:" << final_pkt_buff.Message.data <<std::endl;
                std::cout <<"size of packet list :" << this->ReceivedPackets.size() <<std::endl; 
                //imidiatly send ACK
                struct AckPacket ack;
                ack.type = ACK;
                ack.seq = final_pkt_buff.Message.seq ;
                
                sendto(this->socket_fd, &ack, sizeof(ack), 0, &client_addr, addr_len);
                std::cout<<"ack sent " << ack.seq<< std::endl;

            }
            
            

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
           
            
            
            
            this->mtx.unlock();
        }
    }

    void RegisterReceiver(){
        std::thread listener(&UDP_Server::PacketReceiver , this);
        listener.detach();
    }



void waitForAck(struct sockaddr* client_addr, uint32_t seq) {
    uint16_t tryTimes  = 0 ; 
    std::cout << "in waitfor function" << std::endl ; 
    bool foundAck = false;

    while (!foundAck) {
        this->mtx.lock();
        
        for (int i = 0; i < this->ReceivedAcks.size(); i++) {
            bool isSameAddr = this->ReceivedAcks[i].addr.sin_addr.s_addr ==
                              ((struct sockaddr_in*)client_addr)->sin_addr.s_addr;
            bool isSamePort = this->ReceivedAcks[i].addr.sin_port ==
                              ((struct sockaddr_in*)client_addr)->sin_port;
            bool isSameClient = isSameAddr && isSamePort;

            if (isSameClient && this->ReceivedAcks[i].Ack.seq == seq) {
                std::cout<<"registered the ACK " <<std::endl ; 
                foundAck = true;
                this->ReceivedAcks.erase(this->ReceivedAcks.begin() + i);
                break;
            }
        }
        this->mtx.unlock();

        if (!foundAck) {
            tryTimes +=1 ; 
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // avoid busy loop
            if(tryTimes ==100){
                this->mtx.lock();
                //didnt receive the packet so delete the client from the list 
                for(int i =0 ; i<= this->client_list.size() ; i++){
                    if(((struct sockaddr_in*) & client_addr)->sin_addr.s_addr == ((struct sockaddr_in*) & this->client_list[i])->sin_addr.s_addr){
                        if(((struct sockaddr_in*) & client_addr)->sin_port == ((struct sockaddr_in*) & this->client_list[i])->sin_port){
                            this->client_list.erase(this->client_list.begin()+i);
                        }
                    }
                }
                this->mtx.unlock();
            }
        }
    }

    std::cout << "ACK received for seq " << seq << std::endl;
}

    void broadcast(){
        //std::cout<<"in broadcast()\n" ; 
        //this function should broadcast every received packet to all clients except the sender 
        this->mtx.lock();
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
                    //this->waitForAck(&(this->client_list[j]),this->ReceivedPackets[i].Message.seq );
                    // Run ack listener in a detached thread
                    std::thread ackThread(&UDP_Server::waitForAck, this,
                    &(this->client_list[j]),
                    this->ReceivedPackets[i].Message.seq);
                    ackThread.detach();
                } 
            }
            
                
        }
        this->ReceivedPackets.clear();
        this->mtx.unlock();
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