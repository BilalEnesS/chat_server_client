
# Chat Application

This project implements a simple chat application using C. It consists of two main components: a server (`server5.c`) and a client (`client5.c`). The server can handle multiple client connections simultaneously, allowing users to send messages to each other in real-time.

## Prerequisites

- GCC compiler
- Linux or UNIX environment for `pthread` and network libraries

## Compilation

First, ensure you have GCC installed on your system. You can compile both the server and the client with the following commands:

### Compile the Server

```bash
gcc -o server server5.c -lpthread
```

### Compile the Client

```bash
gcc -o client client5.c -lpthread
```

## Running

After compilation, you can start the server and multiple clients to begin chatting.

### Start the Server

To start the server, use the following command, replacing `<port>` with your desired port number:

```bash
./server <port>
```

### Start a Client

To connect a client to the server, use the following command, replacing `<port>` with the port number used when starting the server:

```bash
./client <port>
```

You will be prompted to enter a name and a password. Note: The current implementation assumes a password check that is not detailed in the provided code snippets. You might want to implement or simulate this functionality according to your requirements.

## Features

- **Multi-threaded server**: Handles multiple clients simultaneously.
- **Client-side message sending and receiving**: Clients can send messages to all other clients connected to the server.
- **Basic authentication**: A password prompt is included for clients to authenticate before joining the chat.

## Limitations

- The server and client lack detailed authentication mechanisms and encryption for messages, making it unsuitable for production environments without further development.

## Contributing

Feel free to fork this project and submit pull requests with improvements or file issues for any bugs you encounter.
