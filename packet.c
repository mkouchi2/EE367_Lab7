/**
@file
@brief Packet sending and receiving implementation

This file provides an implementation for sending and receiving packets between hosts using either pipes or sockets as the underlying communication mechanism.

The implementation includes two main functions:

packet_send(): Sends a packet through the specified network port.
packet_recv(): Receives a packet from the specified network port.
The file depends on the host.h, main.h, net.h, sockets.h, and packet.h header files.

@see host.h
@see main.h
@see net.h
@see sockets.h
@see packet.h
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "host.h"
#include "main.h"
#include "net.h"
#include "sockets.h"
#include "packet.h"

/**
@brief Sends a packet through the specified network port.

The packet_send() function constructs a message from the given packet and sends it through the specified network port using either pipes or sockets, depending on the port type.

@param port Pointer to the net_port structure containing the network port information to send the packet through.
@param p Pointer to the packet structure containing the packet information to send.
*/
void packet_send(struct net_port *port, struct packet *p) {
  char msg[PAYLOAD_MAX + 4];
  int i;

  if (port->type == PIPE) {
    msg[0] = (char)p->src;
    msg[1] = (char)p->dst;
    msg[2] = (char)p->type;
    msg[3] = (char)p->length;
    for (i = 0; i < p->length; i++) {
      msg[i + 4] = p->payload[i];
    }
    write(port->pipe_send_fd, msg, p->length + 4);
  } else if (port->type == SOCKET) {
    struct net_data **g_net_data_ptr = get_g_net_data();
    struct net_data *g_net_data = *g_net_data_ptr;
    create_client(g_net_data->send_domain, g_net_data->send_port, p);
  }

  return;
}

/**
@brief Receives a packet from the specified network port.

The packet_recv() function reads a message from the specified network port and extracts the packet information from it. The function returns the number of bytes received, or a negative value in case of an error.

@param port Pointer to the net_port structure containing the network port information to receive the packet from.
@param p Pointer to the packet structure to store the received packet information.
@return The number of bytes received, or a negative value in case of an error.
*/
int packet_recv(struct net_port *port, struct packet *p) {
  char msg[PAYLOAD_MAX + 4];
  int n;
  int i;

  if (port->type == PIPE) {
    n = read(port->pipe_recv_fd, msg, PAYLOAD_MAX + 4);
  } else if (port->type == SOCKET) {
    struct net_data **g_net_data_ptr = get_g_net_data();
    struct net_data *g_net_data = *g_net_data_ptr;
    n = read(g_net_data->server_pipe, msg, PAYLOAD_MAX + 4);
  }
  if (n > 0) {
    p->src = (char)msg[0];
    p->dst = (char)msg[1];
    p->type = (char)msg[2];
    p->length = (int)msg[3];
    for (i = 0; i < p->length; i++) {
      p->payload[i] = msg[i + 4];
    }
  }

  return (n);
}
