/**
@file switch_util.c
@brief Network switch functions

This file provides a network switch implementation that handles network port information, manages the forwarding table, and sends packets to all associated network ports.

The implementation includes functions to:

\li Display information about a network port.
\li Display the forwarding table.
\li Initialize the forwarding table.
\li Add a source host to the forwarding table.
\li Send a packet to all network ports.
\li Check if a host is in the forwarding table.
\li The file depends on the main.h, packet.h, and switch.h header files.

@see main.h
@see packet.h
@see switch.h
*/

#include <stdio.h>
#include <stdlib.h>

#include "main.h"
#include "packet.h"
#include "switch.h"

/**
@brief Displays the information of a network port.

This function prints the details of the given network port, including its type, pipe host ID, pipe send file descriptor, pipe receive file descriptor, next pointer, and socket host ID.

@param p Pointer to the net_port structure containing the network port information to display.
*/
void display_port_info(struct net_port *p) {
  printf("Net port:\n");
  printf("  type: %d\n", p->type);
  printf("  pipe_host_id: %d\n", p->pipe_host_id);
  printf("  pipe_send_fd: %d\n", p->pipe_send_fd);
  printf("  pipe_recv_fd: %d\n", p->pipe_recv_fd);
  printf("  next: %p\n", (void *)p->next);
  printf("  sock_host_id: %d\n", p->sock_host_id);
}

/**
@brief Displays the forwarding table.

This function prints the contents of the forwarding table, including its size, and for each entry, its validity, host ID, and associated port.

@param table The forward_table structure containing the forwarding table information to display.
*/
void display_forward_table(struct forward_table table) {
  int i;
  printf("Forward table:\n");
  printf("Size: %d\n", table.size);
  printf("Valid\tHost ID\tPort\n");
  for (i = 0; i < 100; i++) {
    if (table.valid[i] != 0 || table.HostID[i] != -1) {
      printf("%d\t%d\t%d\n", table.valid[i], table.HostID[i], table.port[i]);
    }
  }
}

/**
@brief Initializes the forwarding table.

This function initializes the given forwarding table by setting its size to 0 and initializing all entries as invalid with a host ID of -1 and a port of 0.

@param table Pointer to the forward_table structure to initialize.
*/
void init_forward_table(struct forward_table *table) {
  int i;
  table->size = 0;
  for (i = 0; i < MAX_TABLE_SIZE; i++) {
    table->valid[i] = 0;
    table->HostID[i] = -1;
    table->port[i] = 0;
  }
}


/**
@brief Adds a source host to the forwarding table.

This function adds the source host from the given packet to the forwarding table, associating it with the specified port index, only if the host is not already in the table.

@param table Pointer to the forward_table structure to add the source host to.
@param pkt Pointer to the packet structure containing the source host information.
@param port_index The index of the port to associate with the source host in the table.
*/
void add_src_to_table(struct forward_table *table, struct packet *pkt,
                      int port_index) {
  if (table->valid[(int)pkt->src]) return;
  table->valid[(int)pkt->src] = 1;
  table->HostID[(int)pkt->src] = pkt->src;
  table->port[(int)pkt->src] = port_index;
}

/**
@brief Sends a packet to all network ports.

This function sends the given packet to all network ports specified in the node_port array, and then frees the packet memory.

@param node_port_num The number of network ports in the node_port array.
@param node_port Pointer to the array of net_port pointers containing the network ports to send the packet to.
@param pkt Pointer to the packet structure to send.
*/
void send_to_all_ports(int node_port_num, struct net_port **node_port,
                       struct packet *pkt) {
  for (int k = 0; k < node_port_num; k++) {
    packet_send(node_port[k], pkt);
  }
  free(pkt);
}

/**
@brief Checks if a host is in the forwarding table.

This function checks if the specified host is in the given forwarding table by checking the validity of the corresponding entry. It returns 1 if the host is in the table, and 0 otherwise.

@param table Pointer to the forward_table structure to search for the host.
@param dst The host ID to check for in the table.
@return 1 if the host is in the table, 0 otherwise.
*/
int is_host_in_table(struct forward_table *table, char dst) {
  // printf("\ndst=%d table->valid[(intt)dst]=%d\n", dst,
  // table->valid[(int)dst]);
  return table->valid[(int)dst];
}
