
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "main.h"
#include "packet.h"
#include "net.h"
#include "host.h"
#include "sockets.h"

void packet_send(struct net_port *port, struct packet *p)
{
char msg[PAYLOAD_MAX+4];
int i;

if (port->type == PIPE) {
	msg[0] = (char) p->src; 
	msg[1] = (char) p->dst;
	msg[2] = (char) p->type;
	msg[3] = (char) p->length;
	for (i=0; i<p->length; i++) {
		msg[i+4] = p->payload[i];
	}
	write(port->pipe_send_fd, msg, p->length+4);
} else if (port->type == SOCKET) {
   struct net_data **g_net_data_ptr = get_g_net_data();
   struct net_data *g_net_data = *g_net_data_ptr;
   create_client(g_net_data->send_domain, g_net_data->send_port, p);   
}

return;
}

int packet_recv(struct net_port *port, struct packet *p)
{
char msg[PAYLOAD_MAX+4];
int n;
int i;
	
if (port->type == PIPE) {
	n = read(port->pipe_recv_fd, msg, PAYLOAD_MAX+4);
} else if(port->type == SOCKET) {
   struct net_data **g_net_data_ptr = get_g_net_data();
   struct net_data *g_net_data = *g_net_data_ptr;
   n = read(g_net_data->server_pipe, msg, PAYLOAD_MAX+4);

}
   if (n>0) {
		p->src = (char) msg[0];
		p->dst = (char) msg[1];
		p->type = (char) msg[2];
		p->length = (int) msg[3];
		for (i=0; i<p->length; i++) {
			p->payload[i] = msg[i+4];
		}
	}

return(n);
}


