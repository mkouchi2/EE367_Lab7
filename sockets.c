#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>

#include "main.h"
#include "host.h"
#include "net.h"
#include "sockets.h"

void send_packet(int pipe_fd, struct packet *p) {
    char msg[PAYLOAD_MAX+4];
    int i;

    // Convert the packet to a message format
    msg[0] = (char) p->src;
    msg[1] = (char) p->dst;
    msg[2] = (char) p->type;
    msg[3] = (char) p->length;
    for (i=0; i<p->length; i++) {
        msg[i+4] = p->payload[i];
    }

    // Write the message to the pipe
    write(pipe_fd, msg, p->length+4);
}

int receive_packet(int pipe_fd, struct packet *p) {
    char msg[PAYLOAD_MAX+4];
    int i, n;

    // Read the message from the pipe
    n = read(pipe_fd, msg, PAYLOAD_MAX+4);

    // Convert the message to a packet format
    p->src = msg[0];
    p->dst = msg[1];
    p->type = msg[2];
    p->length = (int) msg[3];
    for (i=0; i<p->length; i++) {
        p->payload[i] = msg[i+4];
    }
    p->payload[i] = '\0';
    return n;
}


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
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Set up the address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // Accept a new connection
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Read data from the client and send it to the pipe
        char buffer[PAYLOAD_MAX+4] = {0};
        int valread = read(client_fd, buffer, sizeof(buffer));
        write(pipe_fd, buffer, valread);

        // Close the client socket
        close(client_fd);
    }
}

void create_client(char* domain_name, int port, struct packet* p) {
    int client_fd;
    struct sockaddr_in server_address;


    // Resolve the domain name to an IP address
    struct hostent* he;
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
    server_address.sin_addr = *((struct in_addr*)he->h_addr);
    server_address.sin_port = htons(port);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("connect");
    }

    // Send the packet to the server
    send_packet(client_fd, p);

    // Close the socket
    close(client_fd);
}

//int main() {
//    int fd[2]; // File descriptor array for the pipe
//    char message[] = "Hello, world!"; // Message to be sent through the pipe
//    char buffer[100]; // Buffer to receive the message from the pipe
//    pid_t pid; // Process ID
//
//    // Create the pipe
//    if (pipe(fd) == -1) {
//        perror("pipe");
//        exit(EXIT_FAILURE);
//    }
//
//    // Fork the process
//    pid = fork();
//
//    // Check for errors
//    if (pid < 0) {
//        perror("fork");
//        exit(EXIT_FAILURE);
//    }
//
//    // Child process
//    if (pid == 0) {
//        // Close the read end of the pipe
//        close(fd[0]);
//
//        create_server(3500, fd[1]);
//
//        // Exit the child process
//        exit(EXIT_SUCCESS);
//    }
//
//    // Parent process
//    else {
//        // Close the write end of the pipe
//        close(fd[1]);
//
//        // test packet
//        struct packet *p = malloc(sizeof(struct packet));
//        p->src = 0;
//        p->dst = 1;
//        p->type = PKT_PING_REPLY;
//        p->length = 11;
//        strncpy(p->payload, "Hello World", p->length);
//      
//        create_client("wiliki.eng.hawaii.edu", 3500, p);
//
//        // Read the message from the read end of the pipe
//        struct packet *p2 = malloc(sizeof(struct packet)); 
//        int n = receive_packet(fd[0], p2);
//
//        // diplay the data
//        printf("client process data received = %d\n", n);
//        display_packet_info(p2); 
//        
//        // Close the read end of the pipe
//        close(fd[0]);
//         
//        // kill child process
//        kill(pid, SIGKILL);
//
//        // Exit the parent process
//        exit(EXIT_SUCCESS);
//    }
//}
//
