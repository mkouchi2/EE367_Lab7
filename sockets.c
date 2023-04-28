
/**

@file sockets.c
@brief Provides functions for creating server and client sockets, sending and receiving packets in a C networking program.

This file contains functions for handling sockets in a C networking program, including:

\li Creating a server socket, which listens for incoming connections and forwards received data to a pipe.
\li Creating a client socket, which connects to a remote server and sends packets.
\li Sending a packet, which involves converting the packet to a message format and writing the message to a pipe or socket.
\li Receiving a packet, which involves reading a message from a pipe or socket and converting it to a packet format.

The server socket is created as a child process in the switch.c file, specifically in the switch_main(int host_id) function.

This child process listens for incoming connections, reads data from the clients, and sends the data to the pipe for further

processing by the parent process.

The client socket is used to connect to a remote server and send packets, such as when forwarding packets to their destinations
in a network. The client socket is set to non-blocking mode to allow for asynchronous communication.
*/

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "host.h"
#include "main.h"
#include "net.h"
#include "sockets.h"

/**
@brief Sends a packet by writing its message format to the given pipe.

This function converts the given packet to a message format and writes the message to the specified pipe file descriptor.

@param pipe_fd The file descriptor of the pipe to write the message to.
@param p Pointer to the packet structure to be sent.
*/

void send_packet(int pipe_fd, struct packet *p) {
  char msg[PAYLOAD_MAX + 4];
  int i;

  // Convert the packet to a message format
  msg[0] = (char)p->src;
  msg[1] = (char)p->dst;
  msg[2] = (char)p->type;
  msg[3] = (char)p->length;
  for (i = 0; i < p->length; i++) {
    msg[i + 4] = p->payload[i];
  }

  // Write the message to the pipe
  write(pipe_fd, msg, p->length + 4);
}

/**
@brief Receives a packet by reading its message format from the given pipe.

This function reads a message from the specified pipe file descriptor and converts it to a packet format. It returns the number of bytes read from the pipe.

@param pipe_fd The file descriptor of the pipe to read the message from.
@param p Pointer to the packet structure that will store the received packet data.
@return The number of bytes read from the pipe.
*/
int receive_packet(int pipe_fd, struct packet *p) {
  char msg[PAYLOAD_MAX + 4];
  int i, n;

  // Read the message from the pipe
  n = read(pipe_fd, msg, PAYLOAD_MAX + 4);

  // Convert the message to a packet format
  p->src = msg[0];
  p->dst = msg[1];
  p->type = msg[2];
  p->length = (int)msg[3];
  for (i = 0; i < p->length; i++) {
    p->payload[i] = msg[i + 4];
  }
  p->payload[i] = '\0';
  return n;
}

/**
@brief Creates a server socket, listens for incoming connections and forwards received data to a pipe.

This function creates a server socket, binds it to the specified port, and listens for incoming connections. It accepts connections, reads data from the clients, and sends the data to the specified pipe file descriptor for further processing.

@param port The port number on which the server socket will listen for incoming connections.
@param pipe_fd The file descriptor of the pipe to write the received data to.
*/
void create_server(int port, int pipe_fd) {
  int server_fd, client_fd;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // Create a TCP socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Set the socket options to allow reuse of the address
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  // Set up the address structure
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  // Bind the socket to the address
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  // Start listening for connections
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Accept a new connection
    if ((client_fd = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    // Read data from the client and send it to the pipe
    char buffer[PAYLOAD_MAX + 4] = {0};
    int valread = read(client_fd, buffer, sizeof(buffer));
    write(pipe_fd, buffer, valread);

    // Close the client socket
    close(client_fd);
  }
}

/**
@brief Creates a client socket, connects to a remote server, and sends a packet.

This function creates a client socket, resolves the domain name to an IP address, and connects to the remote server at the specified port. It sets the socket to non-blocking mode for asynchronous communication and sends the given packet to the server.

@param domain_name The domain name of the remote server to connect to.
@param port The port number of the remote server.
@param p Pointer to the packet structure to be sent.
*/
void create_client(char *domain_name, int port, struct packet *p) {
  int client_fd;
  struct sockaddr_in server_address;

  // Resolve the domain name to an IP address
  struct hostent *he;
  he = gethostbyname(domain_name);
  if (he == NULL) {
    perror("gethostbyname");
    exit(EXIT_FAILURE);
  }

  // Create a TCP socket
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }

  // Set the socket to non-blocking mode
  int flags = fcntl(client_fd, F_GETFL, 0);
  if (flags == -1) {
    perror("fcntl");
    exit(EXIT_FAILURE);
  }
  flags |= O_NONBLOCK;
  if (fcntl(client_fd, F_SETFL, flags) == -1) {
    perror("fcntl");
    exit(EXIT_FAILURE);
  }

  // Set up the server address structure
  server_address.sin_family = AF_INET;
  server_address.sin_addr = *((struct in_addr *)he->h_addr);
  server_address.sin_port = htons(port);

  // Connect to the server
  if (connect(client_fd, (struct sockaddr *)&server_address,
              sizeof(server_address)) < 0) {
    perror("connect");
  }

  // Send the packet to the server
  send_packet(client_fd, p);

  // Close the socket
  close(client_fd);
}
