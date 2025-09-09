# voidnet
# UDP Chat System (Work in Progress)

This repository contains a simple **UDP-based chat system** implemented in C++. It includes both a **server** and a **client**, designed to demonstrate concepts such as:

- Socket programming with UDP (`sendto`, `recvfrom`)
- Client management using `sockaddr_in`
- Broadcasting messages to all clients
- Reliable delivery with an **ACK mechanism** (in progress)
- Handling multiple clients with threads

---

## 🚀 Features (Current Status)

### ✅ Implemented
- **Server**
  - Creates a UDP socket and binds to an IP/port.
  - Receives messages from clients.
  - Keeps track of connected clients (`client_list`).
  - Broadcasts received messages to all clients except the sender.
  - Runs a dedicated receiver thread (`RegisterReceiver`).

- **Client**
  - Connects to the server via UDP.
  - Sends an initial handshake (null byte) to register with the server.
  - Receives broadcasted messages in a separate thread.
  - Sends messages typed by the user.
  - Supports `MessagePacket` and `AckPacket` structures.

### 🔧 In Progress
- Implementing a **reliable ACK system** for delivery confirmation.
- Splitting and queueing messages (`addToQueue`).
- Decoder to properly handle input before sending packets.
- Filtering and handling different packet types more robustly.
- Cleaning up buffer handling and error checking.

---

## 🛠️ Build & Run

### Prerequisites
- A Linux/Unix-like environment (uses `<arpa/inet.h>`, `<netinet/in.h>`, `<sys/socket.h>`, etc.)
- `g++` with C++11 or later.

### Compile
```bash
g++ server.cpp -o server -pthread
g++ client.cpp -o client -pthread
