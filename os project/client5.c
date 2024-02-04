#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048 // Maximum message length

// Global variables
volatile sig_atomic_t flag = 0; // Signal flag for interrupt handling
int sockfd = 0; // Socket file descriptor
char name[32]; // Client's name

// Function to overwrite standard output
void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

// Function to trim newline characters
void str_trim_lf(char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}

// Signal handler function for interrupt (SIGINT) signal
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}

// Function to handle sending messages to the server
void send_msg_handler() {
  char message[LENGTH] = {}; // Message buffer
  char buffer[LENGTH + 32] = {}; // Message buffer with sender's name

  while(1) {
    str_overwrite_stdout(); // Overwrite standard output to prompt for input
    fgets(message, LENGTH, stdin); // Read message from standard input
    str_trim_lf(message, LENGTH); // Trim newline characters

    if (strcmp(message, "exit") == 0) { // If the message is "exit", break the loop
      break;
    } else { // Otherwise, format the message with the sender's name and send it to the server
      sprintf(buffer, "%s: %s\n", name, message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

    bzero(message, LENGTH); // Clear the message buffer
    bzero(buffer, LENGTH + 32); // Clear the message buffer with sender's name
  }
  catch_ctrl_c_and_exit(2); // Call the signal handler function to exit
}

// Function to handle receiving messages from the server
void recv_msg_handler() {
  char message[LENGTH] = {}; // Message buffer
  while (1) {
    int receive = recv(sockfd, message, LENGTH, 0); // Receive message from the server
    if (receive > 0) { // If message received successfully, print it
      printf("%s", message);
      str_overwrite_stdout(); // Overwrite standard output to prompt for input
    } else if (receive == 0) { // If connection closed by the server, break the loop
      break;
    } else {
      // Handle receive error (-1)
    }
    memset(message, 0, sizeof(message)); // Clear the message buffer
  }
}

int main(int argc, char **argv){
  if(argc != 2){ // Check if the program is called with the correct number of arguments
    printf("Usage: %s <port>\n", argv[0]); // Print usage message if incorrect
    return EXIT_FAILURE; // Exit with failure status
  }

  char *ip = "127.0.0.1"; // Server IP address
  int port = atoi(argv[1]); // Server port number

  signal(SIGINT, catch_ctrl_c_and_exit); // Register signal handler for interrupt (Ctrl+C) signal

  printf("Please enter your name: "); // Prompt user to enter their name
  fgets(name, 32, stdin); // Read user input for name
  str_trim_lf(name, strlen(name)); // Trim newline characters from the name

  if (strlen(name) > 32 || strlen(name) < 2){ // Check if the name length is valid
    printf("Name must be less than 30 and more than 2 characters.\n"); // Print error message if invalid
    return EXIT_FAILURE; // Exit with failure status
  }

  struct sockaddr_in server_addr; // Server address structure

  /* Socket settings */
  sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
  server_addr.sin_family = AF_INET; // Set address family to IPv4
  server_addr.sin_addr.s_addr = inet_addr(ip); // Set server IP address
  server_addr.sin_port = htons(port); // Set server port number

  // Connect to Server
  int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)); // Connect to server
  if (err == -1) { // Check for connection error
    printf("ERROR: connect\n"); // Print error message if connection failed
    return EXIT_FAILURE; // Exit with failure status
  }

  // Password prompt and verification
  printf("Enter the password: "); // Prompt user to enter the password
  char password[50]; // Password buffer
  fgets(password, 50, stdin); // Read user input for password
  str_trim_lf(password, 50); // Trim newline characters from the password
  send(sockfd, password, strlen(password), 0); // Send password to server

  char server_response[32]; // Server response buffer
  recv(sockfd, server_response, sizeof(server_response), 0); // Receive response from server
  server_response[31] = '\0'; // Ensure null termination of the response

  if (strcmp(server_response, "OK") != 0) { // Check if password verification failed
    printf("Incorrect password. Connection terminated.\n"); // Print error message
    close(sockfd); // Close the socket
    return EXIT_FAILURE; // Exit with failure status
  }

  // Send name to server
  send(sockfd, name, 32, 0); // Send name to server

  printf("=== WELCOME TO THE CHATROOM ===\n"); // Print welcome message

  // Create threads for sending and receiving messages
  pthread_t send_msg_thread; // Thread for sending messages
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
    printf("ERROR: pthread\n"); // Print error message if thread creation fails
    return EXIT_FAILURE; // Exit with failure status
  }

  pthread_t recv_msg_thread; // Thread for receiving messages
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
    printf("ERROR: pthread\n"); // Print error message if thread creation fails
    return EXIT_FAILURE; // Exit with failure status
  }

  while (1){ // Main loop
    if(flag){ // Check if interrupt signal received
      printf("\nBye\n"); // Print exit message
      break; // Break out of the loop
    }
  }

  close(sockfd); // Close the socket

  return EXIT_SUCCESS; // Exit with success status
}
