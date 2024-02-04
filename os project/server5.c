#include <sys/socket.h> // Socket programming functions and structures
#include <netinet/in.h> // Internet address family structures
#include <arpa/inet.h> // Functions for manipulating IP addresses
#include <stdio.h> // Standard input/output functions
#include <stdlib.h> // Standard library functions
#include <unistd.h> // POSIX operating system API
#include <errno.h> // Error handling
#include <string.h> // String manipulation functions
#include <pthread.h> // POSIX threads
#include <sys/types.h> // Data types
#include <signal.h> // Signal handling

#define MAX_CLIENTS 100 // Maximum number of clients allowed
#define BUFFER_SZ 2048 // Size of the buffer for message handling

static _Atomic unsigned int cli_count = 0; // Atomic variable to keep track of the number of clients
static int uid = 10; // Unique ID for each client

/* Client structure */
typedef struct{
    struct sockaddr_in address; // Client's address structure
    int sockfd; // Socket file descriptor for the client
    int uid; // Unique ID of the client
    char name[32]; // Name of the client
} client_t;

client_t *clients[MAX_CLIENTS]; // Array to store client structures

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread safety when accessing client data

void str_overwrite_stdout() {
    printf("\r%s", "> "); // Overwrite stdout to print prompt
    fflush(stdout); // Flush stdout
}

void str_trim_lf (char* arr, int length) {
    int i;
    for (i = 0; i < length; i++) { // Trim newline character from a string
        if (arr[i] == '\n') {
            arr[i] = '\0';
            break;
        }
    }
}

void print_client_addr(struct sockaddr_in addr){
    printf("%d.%d.%d.%d",
        addr.sin_addr.s_addr & 0xff,
        (addr.sin_addr.s_addr & 0xff00) >> 8,
        (addr.sin_addr.s_addr & 0xff0000) >> 16,
        (addr.sin_addr.s_addr & 0xff000000) >> 24);
}

void queue_add(client_t *cl){
    pthread_mutex_lock(&clients_mutex); // Lock the mutex before accessing the client array

    for(int i=0; i < MAX_CLIENTS; ++i){
        if(!clients[i]){ // Find an empty slot in the client array
            clients[i] = cl; // Assign the client structure to the empty slot
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex); // Unlock the mutex after accessing the client array
}

void queue_remove(int uid){
    pthread_mutex_lock(&clients_mutex); // Lock the mutex before accessing the client array

    for(int i=0; i < MAX_CLIENTS; ++i){
        if(clients[i]){ // Iterate through the client array
            if(clients[i]->uid == uid){ // Find the client with the specified UID
                clients[i] = NULL; // Remove the client from the array
                break;
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex); // Unlock the mutex after accessing the client array
}

void send_message(char *s, int uid){
    pthread_mutex_lock(&clients_mutex); // Lock the mutex before accessing the client array

    for(int i=0; i<MAX_CLIENTS; ++i){
        if(clients[i]){ // Iterate through the client array
            if(clients[i]->uid != uid){ // Send message to all clients except the sender
                if(write(clients[i]->sockfd, s, strlen(s)) < 0){ // Write the message to the client's socket
                    perror("ERROR: write to descriptor failed"); // Handle write error
                    break;
                }
            }
        }
    }

    pthread_mutex_unlock(&clients_mutex); // Unlock the mutex after accessing the client array
}

void *handle_client(void *arg){
    char buff_out[BUFFER_SZ]; // Buffer for outgoing messages
    char name[32]; // Buffer for client name
    int leave_flag = 0; // Flag to indicate if the client has left

    client_t *cli = (client_t *)arg; // Extract client structure from argument

    // Password verification
    char password[50]; // Buffer for password
    recv(cli->sockfd, password, 50, 0); // Receive password from the client
    password[49] = '\0'; // Ensure null termination
    str_trim_lf(password, 50); // Trim newline character

    if(strcmp(password, "12345") != 0) { // Check if the password is correct
        printf("Incorrect password from ");
        print_client_addr(cli->address); // Notify about incorrect password
        printf("\n");
        send(cli->sockfd, "FAIL", strlen("FAIL"), 0); // Send failure message to the client
        leave_flag = 1; // Set leave flag to 1
    } else {
        send(cli->sockfd, "OK", strlen("OK"), 0); // Send successful connection message to the client
    }

    if(leave_flag) { // If leave_flag is 1
        close(cli->sockfd); // Close the client socket
        free(cli); // Free the client structure
        pthread_detach(pthread_self()); // Detach the client thread
        return NULL; // Return NULL
    }
    // Notify successful connection
    cli_count++;

    // Name handling
    if(recv(cli->sockfd, name, 32, 0) <= 0 || strlen(name) < 2 || strlen(name) >= 32-1){
        printf("Didn't enter the name.\n"); // Error message if name is not entered
        leave_flag = 1; // Set leave flag to 1
    } else{
        strcpy(cli->name, name); // Copy the name
        sprintf(buff_out, "%s has joined\n", cli->name); // Create the joining message
        printf("%s", buff_out); // Print the message
        send_message(buff_out, cli->uid); // Send the message to all clients
    }

    bzero(buff_out, BUFFER_SZ); // Clear the buffer

    while(1){
        if (leave_flag) { // If leave_flag is 1
            break; // Break the loop
        }

        int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0); // Receive message from client
        if (receive > 0){
            if(strlen(buff_out) > 0){
                send_message(buff_out, cli->uid); // Send the message to all clients

                str_trim_lf(buff_out, strlen(buff_out)); // Remove '\n' character
                printf("%s -> %s\n", buff_out, cli->name); // Print sender and recipient
            }
        } else if (receive == 0 || strcmp(buff_out, "exit") == 0){
            sprintf(buff_out, "%s has left\n", cli->name); // Create the leaving message
            printf("%s", buff_out); // Print the message
            send_message(buff_out, cli->uid); // Send the message to all clients
            leave_flag = 1; // Set leave flag to 1
        } else {
            leave_flag = 1; // Set leave flag to 1
        }

        bzero(buff_out, BUFFER_SZ); // Clear the buffer
    }

    // Cleanup
    close(cli->sockfd); // Close the client socket
    queue_remove(cli->uid); // Remove client from queue
    free(cli); // Free the client structure
    cli_count--; // Decrease client count
    pthread_detach(pthread_self()); // Detach the client thread

    return NULL; // Return NULL
}

int main(int argc, char **argv){
    if(argc != 2){ // Check if the program is called correctly
        printf("Usage: %s <port>\n", argv[0]); // Print the correct usage
        return EXIT_FAILURE; // Exit the program with failure
    }

    char *ip = "127.0.0.1"; // IP address of the server
    int port = atoi(argv[1]); // Port number to use for connection
    int option = 1; // Option for setsockopt function
    int listenfd = 0, connfd = 0; // Listener and connection socket file descriptors
    struct sockaddr_in serv_addr; // Server address
    struct sockaddr_in cli_addr; // Client address
    pthread_t tid; // Thread identifier

    listenfd = socket(AF_INET, SOCK_STREAM, 0); // Create a socket with IPv4 and TCP protocol
    serv_addr.sin_family = AF_INET; // Set the address family
    serv_addr.sin_addr.s_addr = inet_addr(ip); // Set the IP address
    serv_addr.sin_port = htons(port); // Convert port number to network byte order

    signal(SIGPIPE, SIG_IGN); // Ignore the SIGPIPE signal

    if(setsockopt(listenfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0){
        perror("ERROR: setsockopt failed"); // Print error message if there's an error in setsockopt
        return EXIT_FAILURE; // Exit the program with failure
    }

    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR: Socket binding failed"); // Print error message if there's an error in bind
        return EXIT_FAILURE; // Exit the program with failure
    }

    if (listen(listenfd, 10) < 0) {
        perror("ERROR: Socket listening failed"); // Print error message if there's an error in listen
        return EXIT_FAILURE; // Exit the program with failure
    }

    printf("=== WELCOME TO THE CHATROOM ===\n"); // Print welcome message when the server starts

    while(1){ // Start an infinite loop
        socklen_t clilen = sizeof(cli_addr); // Set the size of client address
        connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen); // Accept incoming connection

        if((cli_count + 1) == MAX_CLIENTS){ // If maximum number of clients is reached
            printf("Max clients reached. Rejected: "); // Print message indicating max clients reached
            print_client_addr(cli_addr); // Print the client address
            printf(":%d\n", cli_addr.sin_port); // Print the client port
            close(connfd); // Close the connection
            continue; // Continue to the next iteration of the loop
        }

        client_t *cli = (client_t *)malloc(sizeof(client_t)); // Allocate memory for a new client structure
        cli->address = cli_addr; // Set the client's address
        cli->sockfd = connfd; // Set the client's socket file descriptor
        cli->uid = uid++; // Set the client's unique ID

        queue_add(cli); // Add the client to the queue
        pthread_create(&tid, NULL, &handle_client, (void*)cli); // Create a new thread and start handling the client

        sleep(1); // Wait for 1 second
    }

    return EXIT_SUCCESS; // Exit the program with success
}
