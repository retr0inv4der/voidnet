#include <arpa/inet.h>
#include <cstdint>
#include <cstdio>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>

class UDP_Client {
private:


    int sockfd ;

    struct sockaddr_in dest_addr;

    socklen_t dest_len = sizeof(this->dest_addr) ;

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
    MessagePacket receivedMessage{};
    AckPacket receivedAck{};

    std::vector<struct MessagePacket> queue;

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
        void* buffer[1024];
        sockaddr_in fromAdrr;
        socklen_t fromLen = sizeof(fromAdrr);
        while(true){
            int bytesReceived = recvfrom(sock, buffer, sizeof(buffer), 0 , (struct sockaddr*)&fromAdrr , &fromLen );
            if(bytesReceived>0){
                uint32_t type;
                memcpy(&type, buffer, sizeof(type));

                if(type == ACK){
                    
                    memcpy(&(this->receivedAck), buffer, sizeof(this->receivedAck));
                    
                }else if (type == MESSAGE) {
                    memcpy(&(this->receivedMessage),buffer , sizeof(this->receivedMessage));
                    std::cout<<"[Server]:" << receivedMessage.data << std::endl;
                    std::cout.flush();
                }


            }
        }
    }

    void RegisterReveiver(){
        std::thread listener(&UDP_Client::receiveMessages ,this ,  this->sockfd);
        listener.detach();
    }
    void initMessage(){
        //this function sends a nullbyte to the server for initialization purposes at the begining of the program
        char message ='\0';
        int n = sendto(this->sockfd, &message, sizeof(message), 0, (struct sockaddr*)&(this->dest_addr) ,this->dest_len );
        if (n < 0) {
            perror("sendto(init message) failed");
        }
    }


    void addToQueue(std::string msg){
        //we should split the message and add the packets to the queue
        // implement the decoder here mr tom5596

    
    }


    void SendPacket(){
        MessagePacket Message{} ; 
        this->receivedAck.seq =0;
        int seqNum =1;
        for(int i =0 ; i<this->queue.size() ; i++){
            Message = this->queue[i];
            while(!this->receivedAck.seq){
                sendto(this->sockfd, &Message, sizeof(Message), 0, (struct sockaddr*)&this->dest_addr, this->dest_len);
                if(this->receivedAck.seq ==Message.seq) break; //ack
            }
            
        }   
        this->queue.clear(); 
        this->receivedAck.seq = 0;
    }
        
    
    void start(){
            this->initMessage();
            this->RegisterReveiver();

            std::string message; 
            while (true) {
            std::cout << " > " ; 
            std::getline(std::cin , message); // TODO: HANDLE THE INPUT WITH THE DECODER (tom5596)
            //TODO: ADD TO QUEUE (tom5596)
            this->SendPacket();
        }
    }
};




int main(){
    char ip[] = "127.0.0.1";
    UDP_Client client  = UDP_Client(ip , 3000);
    client.start();
}