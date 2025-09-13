# voidnet
### UDP Chat System (Work in Progress)

This project is a **UDP-based chat system** written in C++.  
It provides a lightweight server and client that communicate over UDP sockets while experimenting with **reliability mechanisms** such as acknowledgments (ACKs).

---

## ✨ Features

### ✅ Implemented
- **Server**
  - Creates a UDP socket and binds to a given IP/port.
  - Receives `MessagePacket` and `AckPacket` structures from clients.
  - Tracks connected clients (`client_list`).
  - Broadcasts messages to all clients except the sender.
  - Immediately sends ACKs back to the message sender.
  - Runs a dedicated packet listener thread (`RegisterReceiver`).

- **Client**
  - Connects to the server using UDP.
  - Sends an initial handshake (null byte) for registration.
  - Listens for incoming messages in a background thread.
  - Prints server-broadcasted messages.
  - Sends messages typed by the user.
  - Implements ACK sending when receiving `MessagePacket`s.

### 🔧 In Progress
- Full **reliable ACK system** to guarantee delivery.
- Queueing and splitting long messages (`addToQueue` + `mheader` integration).
- Decoder for handling structured input before sending packets.
- Better buffer and error handling.
- More robust client management (timeouts, disconnects).

---

## 📦 Packet Structures

```cpp
struct MessagePacket {
    uint32_t type;   // MESSAGE
    uint32_t seq;    // Sequence number
    uint32_t size;   // Payload size
    char data[256];  // Message
};

struct AckPacket {
    uint32_t type;   // ACK
    uint32_t seq;    // Sequence number being acknowledged
};
```

## 🛠️ Build & Run
- **Requirements**

  - Linux / Unix-like environment

  - g++ with C++11 or later

  - POSIX sockets (arpa/inet.h, netinet/in.h, sys/socket.h)

**compile**
```bash
g++ server.cpp -o server -pthread
g++ client.cpp -o client -pthread
```

**Run**

  - Start the server:
```bash
./server
```

  - Run one or more clients:
```bash
./client
```

Clients can now send messages, which the server broadcasts to all connected clients (excluding the sender).