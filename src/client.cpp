#include "./mheader/mheader.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/struct_timeval.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <ostream>
#include <queue>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

class UDP_Client {
private:
  int sockfd;

  struct sockaddr_in dest_addr;

  socklen_t dest_len = sizeof(this->dest_addr);

  enum PacketType { MESSAGE = 1, ACK = 2 };

  struct MessagePacket {
    uint32_t type; // MESSAGE
    uint32_t seq;
    uint32_t size;
    char data[256];
  };

  struct AckPacket {
    uint32_t type; // ACK
    uint32_t seq;
  };
  MessagePacket receivedMessage{};
  AckPacket receivedAck{};

  std::vector<struct MessagePacket> queue;
  std::vector<struct MessagePacket> receivedMessages;
  std::mutex mtx;

public:
  UDP_Client(char ip_addr[], int port) {
    // this function inits the socketfd and then sets the dest address
    // 1. init the  socket
    if ((this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
      perror("failed in socket creation");
      exit(1);
    }

    // 2. set the dest address
    this->dest_addr.sin_family = AF_INET;
    this->dest_addr.sin_port = htons(port);
    this->dest_addr.sin_addr.s_addr = inet_addr(ip_addr);
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv,
               sizeof(tv));
  }

  void receiveMessages(int sock) {
    char buffer[1024];
    sockaddr_in fromAdrr;
    socklen_t fromLen = sizeof(fromAdrr);
    while (true) {

      int bytesReceived = recvfrom(sock, buffer, sizeof(buffer), 0,
                                   (struct sockaddr *)&fromAdrr, &fromLen);
      if (bytesReceived > 0) {
        this->mtx.lock();
        uint32_t type;
        memcpy(&type, buffer, sizeof(type));

        if (type == ACK) {

          memcpy(&(this->receivedAck), buffer, sizeof(this->receivedAck));
          std::cout << "[DEBUG] : from: receiveMEssages()---> recieved:"
                    << this->receivedAck.seq;

        } else if (type == MESSAGE) {
          memcpy(&(this->receivedMessage), buffer,
                 sizeof(this->receivedMessage));
          //-----send ack -----
          struct AckPacket ack;
          ack.type = ACK;
          ack.seq = this->receivedMessage.seq;
          sendto(this->sockfd, &ack, sizeof(ack), 0,
                 (struct sockaddr *)&this->dest_addr, this->dest_len);
          //-----ack sent-----

          this->receivedMessages.push_back(this->receivedMessage);
          // check if the message is the last one in the flow
          std::cout << "[Server]:";
          // DEBUG
          std::cout << "DEBUG ---> recieved SEQ =" << this->receivedMessage.seq
                    << " received SIZE: " << this->receivedMessage.size
                    << std::endl;
          if (this->receivedMessage.seq == this->receivedMessage.size) {
            for (int i = 0; i < this->receivedMessages.size(); i++) {
              std::cout << this->receivedMessages[i].data;
              std::cout.flush();
            }
            this->receivedMessages.clear();
          }
        }

        mtx.unlock();
      }
      // unlock the mutex after each loop
    }
  }

  void RegisterReveiver() {
    std::thread listener(&UDP_Client::receiveMessages, this, this->sockfd);
    listener.detach();
  }
  void initMessage() {
    // this function sends a nullbyte to the server for initialization purposes
    // at the begining of the program
    char message = '\0';
    int n = sendto(this->sockfd, &message, sizeof(message), 0,
                   (struct sockaddr *)&(this->dest_addr), this->dest_len);
    if (n < 0) {
      perror("sendto(init message) failed");
    }
  }

  void addToQueue(std::string msg) {
    // we should split the message and add the packets to the queue
    //  implement the decoder here mr tom5596

    // hello its me, mr tom! :P

    std::queue<mHeader *> mHeaderQueue;

    mheader_ls hdr_list =
        encode_mheader((char *)msg.c_str(), msg.size(), MHEADER_MESSAGE);
    for (int i = 0; i < hdr_list.size(); i++) {

      mHeaderQueue.push(hdr_list[i]);
    }

    // for debugging
    size_t size = mHeaderQueue.size();
    for (int i = 0; i < size; i++) {
      std::cout << std::endl
                << "[DEBUG]" << "mheader size: " << mHeaderQueue.size()
                << "   mHeaderQueue.front() -> " << mHeaderQueue.front()->data
                << std::endl;
      struct MessagePacket msg_pckt;
      msg_pckt.type = MESSAGE;
      // copy the data from mheader.front to the data member of the message
      // packet
      memcpy(msg_pckt.data, &(mHeaderQueue.front()->data), 256);
      msg_pckt.seq = mHeaderQueue.front()->seq + 1;
      msg_pckt.size = size;

      // add the message to the queue
      this->queue.push_back(msg_pckt);

      // delete the item from the mheader queue
      mHeaderQueue.pop();
    }
  }

  void waitForAck(uint32_t seq) {
    fd_set readfds;
    struct timeval timeout;
    int retval;

    while (true) {
      // set up the file descriptor set
      FD_ZERO(&readfds);
      FD_SET(this->sockfd, &readfds);

      // set timeout
      timeout.tv_sec = 2;
      timeout.tv_usec = 0;

      // wait for data to be ready or timeout
      retval = select(this->sockfd + 1, &readfds, NULL, NULL, &timeout);

      if (retval == -1) {
        perror("select()");
        exit(1);
      } else if (retval == 0) {
        std::cout << "Connection timed out. Disconnecting..." << std::endl;
        close(this->sockfd);
        exit(1);
      }

      // check if we received the expected ack
      if (this->receivedAck.seq == seq) {
        std::cout << "[DEBUG] : from: waitForAck() registered:"
                  << this->receivedAck.seq;
        break; // Acknowledgment received
      }
    }
  }

  void SendPacket() {

    this->mtx.lock();

    MessagePacket Message{};
    this->receivedAck.seq = 0;
    int seqNum = 1;
    for (int i = 0; i < this->queue.size(); i++) {
      Message = this->queue[i];

      sendto(this->sockfd, &Message, sizeof(Message), 0,
             (struct sockaddr *)&this->dest_addr, this->dest_len);

      // this->waitForAck(Message.seq ) ;
    }
    this->queue.clear();
    this->mtx.unlock();
    std::cout << "---- unlocked ----\n";
  }

  void start() {
    this->initMessage();
    this->RegisterReveiver();

    std::string message;
    while (true) {
      std::cout << " > ";
      std::getline(
          std::cin,
          message); // TODO: HANDLE THE INPUT WITH THE DECODER (tom5596)
      // TODO: ADD TO QUEUE (tom5596)
      this->addToQueue(message);

      this->SendPacket();
    }
  }
};

int main() {
  char ip[] = "127.0.0.1";
  UDP_Client client = UDP_Client(ip, 3000);
  client.start();
}
