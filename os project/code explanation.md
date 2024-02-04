
# Chat Application Code Explanation

This document provides a detailed explanation of the server and client code for the chat application implemented in C.

## Server Code (`server5.c`)

The server code handles multiple client connections, manages client sessions, and broadcasts messages from one client to all other clients.

### Key Components

- **Socket Programming**: Utilizes the POSIX sockets API for network communication.
- **Multi-threading**: Uses `pthread` library to handle multiple clients concurrently.
- **Client Management**: Maintains a dynamic list of clients and supports adding/removing clients.
- **Message Broadcasting**: Receives messages from any client and sends them to all other connected clients.

### Functions

- `void str_overwrite_stdout()`: Clears the current line in the terminal.
- `void str_trim_lf(char* arr, int length)`: Trims newline characters from a string.
- `void print_client_addr(struct sockaddr_in addr)`: Prints the IP address of a client.
- `void queue_add(client_t *cl)`: Adds a client to the list of managed clients.
- `void queue_remove(int uid)`: Removes a client from the list based on its unique identifier.
- `void send_message(char *s, int uid)`: Broadcasts a message to all clients except the sender.
- `void *handle_client(void *arg)`: Handles incoming messages from a client and broadcasts them.

### Main Function

- Initializes the server socket and binds it to an IP and port.
- Listens for incoming connections and accepts them.
- For each accepted connection, allocates a client structure, and spawns a new thread to handle communication with the client.

## Client Code (`client5.c`)

The client code connects to the server, sends messages, and receives broadcasts from other clients.

### Key Components

- **Socket Programming**: Uses POSIX sockets API for connecting to the server.
- **Multi-threading**: Employs `pthread` for handling sending and receiving messages concurrently.
- **Signal Handling**: Catches `SIGINT` (Ctrl+C) for graceful disconnection.

### Functions

- `void str_overwrite_stdout()`: Clears the current line in the terminal for user input.
- `void str_trim_lf(char* arr, int length)`: Trims newline characters from strings.
- `void catch_ctrl_c_and_exit(int sig)`: Signal handler for Ctrl+C.
- `void send_msg_handler()`: Reads user input and sends messages to the server.
- `void recv_msg_handler()`: Receives messages from the server and prints them.

### Main Function

- Prompts the user for a name and connects to the server using the provided IP and port.
- Initiates password verification with the server.
- Creates threads for handling sending and receiving messages.

## Compilation and Execution

Refer to the main README.md for instructions on compiling and running the server and client.

## Conclusion

This chat application demonstrates fundamental concepts of network programming, multi-threaded programming, and client-server architecture in C.
