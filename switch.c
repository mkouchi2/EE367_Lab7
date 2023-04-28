/**
@file switch.c

@brief Main program for managing network port communication, forwarding table, and packet handling.

This file contains the main program for a network switch application, handling network port communication, maintaining a forwarding table, and processing incoming packets. The main functionality includes:

\li Displaying network port information.
\li Displaying and managing the forwarding table.
\li Initializing the forwarding table.
\li Adding source hosts to the forwarding table.
\li Sending packets to all network ports.
\li Checking if a host is in the forwarding table.

The main program manages the forwarding table, which is used to keep track of host IDs and their associated ports. When a packet is received, the main program checks if the destination host is in the forwarding table. If so, it forwards the packet to the appropriate port. If not, it broadcasts the packet to all network ports.

*/


#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "host.h"
#include "main.h"
#include "man.h"
#include "net.h"
#include "packet.h"
#include "sockets.h"
#include "switch.h"
#include "switch_util.h"

void switch_main(int host_id) {
  // initialization
  struct net_port *node_port_list;
  struct net_port **node_port;
  int node_port_num;
  struct packet *in_packet;
  struct packet *new_packet;
  struct packet *tree_packet;
  struct net_port *p;
  int i, k, n;
  struct forward_table table;

  init_forward_table(&table);

  node_port_list = net_get_port_list(host_id);

  node_port_num = 0;
  for (p = node_port_list; p != NULL; p = p->next) {
    node_port_num++;
  }

  node_port =
      (struct net_port **)malloc(node_port_num * sizeof(struct net_port *));

  // populate node_port
  p = node_port_list;
  for (k = 0; k < node_port_num; k++) {
    node_port[k] = p;
    p = p->next;
  }

  // display_forward_table(table);

  // socket
  int fd[2];
  pid_t pid;
  struct net_data **g_net_data_ptr = get_g_net_data();
  struct net_data *g_net_data = *g_net_data_ptr;

  // Create the pipe
  if (pipe(fd) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  // Set the read end of the pipe to non-blocking
  if (fcntl(fd[0], F_SETFL, O_NONBLOCK) == -1) {
    perror("fcntl");
    exit(EXIT_FAILURE);
  }

  // Set the write end of the pipe to non-blocking
  if (fcntl(fd[1], F_SETFL, O_NONBLOCK) == -1) {
    perror("fcntl");
    exit(EXIT_FAILURE);
  }

  // Fork the process
  pid = fork();

  // Check for errors
  if (pid < 0) {
    perror("fork");
    exit(EXIT_FAILURE);
  }

  // Child process
  if (pid == 0) {
    // Close the read end of the pipe
    close(fd[0]);

    create_server(g_net_data->server_port, fd[1]);

    // Exit the child process
    exit(EXIT_SUCCESS);
  }

  // Parent process
  else {
    // Close the write end of the pipe
    close(fd[1]);
    g_net_data->server_pipe = fd[0];

    // main loop
    while (1) {
      // get packets from incoming links
      // scan all ports
      for (k = 0; k < node_port_num; k++) {
        in_packet = (struct packet *)malloc(sizeof(struct packet));
        n = packet_recv(node_port[k], in_packet);

        if (n > 0) {
          // display_packet_info(in_packet);
          //              display_forward_table(table);
          // add packet routing here
          // check whole table
          if (is_host_in_table(&table, in_packet->dst)) {
            // port is in table, send it
            packet_send(node_port[table.port[in_packet->dst]], in_packet);
            add_src_to_table(&table, in_packet, k);

          } else {
            // port is not in table
            add_src_to_table(&table, in_packet, k);
            send_to_all_ports(node_port_num, node_port, in_packet);
          }
        }
      }
    }
  }
}
