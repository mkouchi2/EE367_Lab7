
/**
 *@file net.c

 * @brief This file initializes the network.

This file handles the initialization and management of a network, 
including nodes, links, and communication between hosts and a manager. 
It provides various functions for network manipulation, such as creating 
nodes, links, and port lists, as well as loading a network configuration 
file.

*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "host.h"
#include "main.h"
#include "man.h"
#include "packet.h"
#include "net.h"

/*
 * The following are private global variables to this file net.c
 */
static enum bool g_initialized = FALSE; /* Network initialized? */
/* The network is initialized only once */

/*
 * g_net_node[] and g_net_node_num have link information from
 * the network configuration file.
 * g_node_list is a linked list version of g_net_node[]
 */
static struct net_node *g_net_node;
static int g_net_node_num;
static struct net_node *g_node_list = NULL;

/*
 * g_net_link[] and g_net_link_num have link information from
 * the network configuration file
 */
static struct net_link *g_net_link;
static int g_net_link_num;

/*
 * Global private variables about ports of network node links
 * and ports of links to the manager
 */
static struct net_port *g_port_list = NULL;

static struct man_port_at_man *g_man_man_port_list = NULL;
static struct man_port_at_host *g_man_host_port_list = NULL;

static struct net_data *g_net_data = NULL;

struct net_data **get_g_net_data() { return &g_net_data; }
/*
 * Loads network configuration file and creates data structures
 * for nodes and links.  The results are accessible through
 * the private global variables
 */
int load_net_data_file();

/*
 * Creates a data structure for the nodes
 */
void create_node_list();

/*
 * Creates links, using pipes
 * Then creates a port list for these links.
 */
void create_port_list();

/*
 * Creates ports at the manager and ports at the hosts so that
 * the manager can communicate with the hosts.  The list of
 * ports at the manager side is p_m.  The list of ports
 * at the host side is p_h.
 */
void create_man_ports(struct man_port_at_man **p_m,
                      struct man_port_at_host **p_h);

void net_close_man_ports_at_hosts();
void net_close_man_ports_at_hosts_except(int host_id);
void net_free_man_ports_at_hosts();
void net_close_man_ports_at_man();
void net_free_man_ports_at_man();

/*
 * Get the list of ports for host host_id
 */
struct net_port *net_get_port_list(int host_id);

/*
 * Get the list of nodes
 */
struct net_node *net_get_node_list();

/**
 * @brief Retrieves a list of net_port structures for a given host ID.
 *
 * This function retrieves a list of net_port structures that match the
 * specified host ID, either as pipe_host_id or sock_host_id. The function
 * modifies the global port list and creates a new list with the matching
 * net_port structures.
 *
 * @param host_id The ID of the host to search for.
 * @return A pointer to the first net_port structure in the list of matching
 *         ports, or NULL if no matching ports are found.
 */
struct net_port *net_get_port_list(int host_id) {
  struct net_port **p;
  struct net_port *r;
  struct net_port *t;

  r = NULL;
  p = &g_port_list;

  while (*p != NULL) {
    if ((*p)->pipe_host_id == host_id || (*p)->sock_host_id == host_id) {
      t = *p;
      *p = (*p)->next;
      t->next = r;
      r = t;
    } else {
      p = &((*p)->next);
    }
  }

  return r;
}

/** Return the linked list of nodes */
struct net_node *net_get_node_list() { return g_node_list; }

/**
 * @brief Retrieves the list of management ports at the management node.
 *
 * This function returns the global list of management ports at the management
 * node.
 *
 * @return A pointer to the first man_port_at_man structure in the list.
 */struct man_port_at_man *net_get_man_ports_at_man_list() {
  return (g_man_man_port_list);
}

/**
 * @brief Retrieves the management port at a host with a given host ID.
 *
 * This function searches for the management port at a host with the specified
 * host ID and returns it.
 *
 * @param host_id The ID of the host to search for.
 * @return A pointer to the man_port_at_host structure, or NULL if not found.
 */
struct man_port_at_host *net_get_host_port(int host_id) {
  struct man_port_at_host *p;

  for (p = g_man_host_port_list; p != NULL && p->host_id != host_id;
       p = p->next)
    ;

  return (p);
}

/**
 * @brief Closes all management ports at host nodes.
 *
 * This function iterates through the list of management ports at host nodes
 * and closes both the send and receive file descriptors for each port.
 */
void net_close_man_ports_at_hosts() {
  struct man_port_at_host *p_h;

  p_h = g_man_host_port_list;

  while (p_h != NULL) {
    close(p_h->send_fd);
    close(p_h->recv_fd);
    p_h = p_h->next;
  }
}

/**
 * @brief Closes all management ports at host nodes, except for the specified host ID.
 *
 * This function iterates through the list of management ports at host nodes
 * and closes both the send and receive file descriptors for each port, except
 * for the one with the specified host ID.
 *
 * @param host_id The ID of the host to exclude from closing.
 */
void net_close_man_ports_at_hosts_except(int host_id) {
  struct man_port_at_host *p_h;

  p_h = g_man_host_port_list;

  while (p_h != NULL) {
    if (p_h->host_id != host_id) {
      close(p_h->send_fd);
      close(p_h->recv_fd);
    }
    p_h = p_h->next;
  }
}

/**
 * @brief Frees the memory allocated for management ports at host nodes.
 *
 * This function iterates through the list of management ports at host nodes
 * and frees the memory allocated for each man_port_at_host structure.
 */
void net_free_man_ports_at_hosts() {
  struct man_port_at_host *p_h;
  struct man_port_at_host *t_h;

  p_h = g_man_host_port_list;

  while (p_h != NULL) {
    t_h = p_h;
    p_h = p_h->next;
    free(t_h);
  }
}

/**
 * @brief Closes all management ports at the management node.
 *
 * This function iterates through the list of management ports at the management
 * node and closes both the send and receive file descriptors for each port.
 */
void net_close_man_ports_at_man() {
  struct man_port_at_man *p_m;

  p_m = g_man_man_port_list;

  while (p_m != NULL) {
    close(p_m->send_fd);
    close(p_m->recv_fd);
    p_m = p_m->next;
  }
}

/**
 * @brief Frees the memory allocated for management ports at the management node.
 *
 * This function iterates through the list of management ports at the management
 * node and frees the memory allocated for each man_port_at_man structure.
 */
void net_free_man_ports_at_man() {
  struct man_port_at_man *p_m;
  struct man_port_at_man *t_m;

  p_m = g_man_man_port_list;

  while (p_m != NULL) {
    t_m = p_m;
    p_m = p_m->next;
    free(t_m);
  }
}

/* Initialize network ports and links */
int net_init() {
  if (g_initialized == TRUE) { /* Check if the network is already initialized */
    printf("Network already loaded\n");
    return (0);
  } else if (load_net_data_file() == 0) { /* Load network configuration file */
    return (0);
  }
  /*
   * Create a linked list of node information at g_node_list
   */
  create_node_list();

  /*
   * Create pipes and sockets to realize network links
   * and store the ports of the links at g_port_list
   */
  create_port_list();

  /*
   * Create pipes to connect the manager to hosts
   * and store the ports at the host at g_man_host_port_list
   * as a linked list
   * and store the ports at the manager at g_man_man_port_list
   * as a linked list
   */
  create_man_ports(&g_man_man_port_list, &g_man_host_port_list);
}

/**
 * @brief Creates management ports for host nodes and the management node.
 *
 * This function creates management ports for host nodes and the management
 * node by iterating through the list of net_node structures. It sets up the
 * pipes, file descriptors, and adds the created ports to the corresponding
 * global lists.
 *
 * @param[out] p_man Pointer to the pointer of the first man_port_at_man structure.
 * @param[out] p_host Pointer to the pointer of the first man_port_at_host structure.
 */
void create_man_ports(struct man_port_at_man **p_man,
                      struct man_port_at_host **p_host) {
  struct net_node *p;
  int fd0[2];
  int fd1[2];
  struct man_port_at_man *p_m;
  struct man_port_at_host *p_h;
  int host;

  for (p = g_node_list; p != NULL; p = p->next) {
    if (p->type == HOST) {
      p_m = (struct man_port_at_man *)malloc(sizeof(struct man_port_at_man));
      p_m->host_id = p->id;

      p_h = (struct man_port_at_host *)malloc(sizeof(struct man_port_at_host));
      p_h->host_id = p->id;

      pipe(fd0); /* Create a pipe */
                 /* Make the pipe nonblocking at both ends */
      fcntl(fd0[PIPE_WRITE], F_SETFL,
            fcntl(fd0[PIPE_WRITE], F_GETFL) | O_NONBLOCK);
      fcntl(fd0[PIPE_READ], F_SETFL,
            fcntl(fd0[PIPE_READ], F_GETFL) | O_NONBLOCK);
      p_m->send_fd = fd0[PIPE_WRITE];
      p_h->recv_fd = fd0[PIPE_READ];

      pipe(fd1); /* Create a pipe */
                 /* Make the pipe nonblocking at both ends */
      fcntl(fd1[PIPE_WRITE], F_SETFL,
            fcntl(fd1[PIPE_WRITE], F_GETFL) | O_NONBLOCK);
      fcntl(fd1[PIPE_READ], F_SETFL,
            fcntl(fd1[PIPE_READ], F_GETFL) | O_NONBLOCK);
      p_h->send_fd = fd1[PIPE_WRITE];
      p_m->recv_fd = fd1[PIPE_READ];

      p_m->next = *p_man;
      *p_man = p_m;

      p_h->next = *p_host;
      *p_host = p_h;
    }
  }
}

/**

\brief Creates a node list based on the network configuration.

This function reads the global network node data and creates a linked list
of net_node structures, which represents the network nodes.
The resulting linked list is stored in the global variable g_node_list.
*/
void create_node_list() {
  struct net_node *p;
  int i;

  g_node_list = NULL;
  for (i = 0; i < g_net_node_num; i++) {
    p = (struct net_node *)malloc(sizeof(struct net_node));
    p->id = g_net_node[i].id;
    p->type = g_net_node[i].type;
    p->next = g_node_list;
    g_node_list = p;
  }
}

/**

\brief Creates a port list based on the network configuration.

This function reads the global network link data and creates a linked list
of net_port structures, which represents the network ports.
The resulting linked list is stored in the global variable g_port_list.
Each port in the list is properly configured according to its type (PIPE or SOCKET),
and it also includes necessary file descriptors for sending and receiving data.
*/
void create_port_list() {
  struct net_port *p0;
  struct net_port *p1;
  int node0, node1;
  int fd01[2];
  int fd10[2];
  int i;

  g_port_list = NULL;
  for (i = 0; i < g_net_link_num; i++) {
    if (g_net_link[i].type == PIPE) {
      node0 = g_net_link[i].pipe_node0;
      node1 = g_net_link[i].pipe_node1;

      p0 = (struct net_port *)malloc(sizeof(struct net_port));
      p0->type = g_net_link[i].type;
      p0->pipe_host_id = node0;

      p1 = (struct net_port *)malloc(sizeof(struct net_port));
      p1->type = g_net_link[i].type;
      p1->pipe_host_id = node1;

      pipe(fd01); /* Create a pipe */
                  /* Make the pipe nonblocking at both ends */
      fcntl(fd01[PIPE_WRITE], F_SETFL,
            fcntl(fd01[PIPE_WRITE], F_GETFL) | O_NONBLOCK);
      fcntl(fd01[PIPE_READ], F_SETFL,
            fcntl(fd01[PIPE_READ], F_GETFL) | O_NONBLOCK);
      p0->pipe_send_fd = fd01[PIPE_WRITE];
      p1->pipe_recv_fd = fd01[PIPE_READ];

      pipe(fd10); /* Create a pipe */
                  /* Make the pipe nonblocking at both ends */
      fcntl(fd10[PIPE_WRITE], F_SETFL,
            fcntl(fd10[PIPE_WRITE], F_GETFL) | O_NONBLOCK);
      fcntl(fd10[PIPE_READ], F_SETFL,
            fcntl(fd10[PIPE_READ], F_GETFL) | O_NONBLOCK);
      p1->pipe_send_fd = fd10[PIPE_WRITE];
      p0->pipe_recv_fd = fd10[PIPE_READ];

      p0->sock_host_id = -1;
      p1->sock_host_id = -1;

      p0->next = p1; /* Insert ports in linked lisst */
      p1->next = g_port_list;
      g_port_list = p0;

    } else if (g_net_link[i].type == SOCKET) {
      p0 = (struct net_port *)malloc(sizeof(struct net_port));
      p0->type = g_net_link[i].type;
      p0->sock_host_id = g_net_data->switch_host_id;

      p0->next = g_port_list;
      g_port_list = p0;
    }
  }
}

/**
\brief Loads network data from a file and initializes the network structures.

This function prompts the user for a network data file, reads the file, and
initializes the network structures (nodes and links) based on the file content.
It stores the number of nodes and links in the private global variables g_net_node_num
and g_net_link_num, respectively. The nodes and links are stored in global arrays
g_net_node and g_net_link, respectively.

\return 1 if the data file is loaded successfully and the network structures are
initialized correctly, 0 otherwise.
*/
int load_net_data_file() {
  FILE *fp;
  char fname[MAX_FILE_NAME];
  g_net_data = malloc(sizeof(struct net_data));

  /* Open network configuration file */
  printf("Enter network data file: ");
  scanf("%s", fname);
  fp = fopen(fname, "r");
  if (fp == NULL) {
    printf("net.c: File did not open\n");
    return (0);
  }

  int i;
  int node_num;
  char node_type;
  int node_id;

  /*
   * Read node information from the file and
   * fill in data structure for nodes.
   * The data structure is an array g_net_node[ ]
   * and the size of the array is g_net_node_num.
   * Note that g_net_node[] and g_net_node_num are
   * private global variables.
   */
  fscanf(fp, "%d", &node_num);
  printf("Number of Nodes = %d: \n", node_num);
  g_net_node_num = node_num;

  if (node_num < 1) {
    printf("net.c: No nodes\n");
    fclose(fp);
    return (0);
  } else {
    g_net_node = (struct net_node *)malloc(sizeof(struct net_node) * node_num);
    for (i = 0; i < node_num; i++) {
      fscanf(fp, " %c ", &node_type);
      if (node_type == 'H') {
        fscanf(fp, " %d ", &node_id);
        g_net_node[i].type = HOST;
        g_net_node[i].id = node_id;
      } else if (node_type == 'S') {
        fscanf(fp, " %d ", &node_id);
        g_net_node[i].type = SWITCH;
        g_net_node[i].id = node_id;
        g_net_data->switch_host_id = node_id;
      }

      else {
        printf(" net.c: Unidentified Node Type\n");
      }
    }
  }

  /*
   * Read link information from the file and
   * fill in data structure for links.
   * The data structure is an array g_net_link[ ]
   * and the size of the array is g_net_link_num.
   * Note that g_net_link[] and g_net_link_num are
   * private global variables.
   */

  int link_num;
  char link_type;
  int node0, node1;
  int server_port;
  int send_port;
  char send_domain[100];
  char server_domain[100];

  fscanf(fp, " %d ", &link_num);
  printf("Number of links = %d\n", link_num);
  g_net_link_num = link_num;

  if (link_num < 1) {
    printf("net.c: No links\n");
    fclose(fp);
    return (0);
  } else {
    g_net_link = (struct net_link *)malloc(sizeof(struct net_link) * link_num);
    for (i = 0; i < link_num; i++) {
      fscanf(fp, " %c ", &link_type);
      if (link_type == 'P') {
        fscanf(fp, " %d %d ", &node0, &node1);
        g_net_link[i].type = PIPE;
        g_net_link[i].pipe_node0 = node0;
        g_net_link[i].pipe_node1 = node1;
      } else if (link_type == 'S') {
        fscanf(fp, " %d %s %d %s %d ", &node0, send_domain, &send_port,
               server_domain, &server_port);
        g_net_link[i].type = SOCKET;

        g_net_data->send_port = send_port;
        g_net_data->server_port = server_port;
        strcpy(g_net_data->send_domain, send_domain);
        // using indexed links w/o breaking maxwells g_net_data strucutre
        g_net_link[i].send_port = send_port;
        g_net_link[i].server_port = server_port;
        strcpy(g_net_link[i].send_domain, send_domain);
        strcpy(g_net_link[i].server_domain, server_domain);
      }

      else {
        printf("   net.c: Unidentified link type\n");
      }
    }
  }

  /* Display the nodes and links of the network */
  printf("Nodes:\n");
  for (i = 0; i < g_net_node_num; i++) {
    if (g_net_node[i].type == HOST) {
      printf("   Node %d HOST\n", g_net_node[i].id);
    } else if (g_net_node[i].type == SWITCH) {
      printf("   Node %d SWITCH\n", g_net_node[i].id);
    } else {
      printf(" Unknown Type\n");
    }
  }
  printf("Links:\n");
  for (i = 0; i < g_net_link_num; i++) {
    if (g_net_link[i].type == PIPE) {
      printf("   Link (%d, %d) PIPE\n", g_net_link[i].pipe_node0,
             g_net_link[i].pipe_node1);
    } else if (g_net_link[i].type == SOCKET) {
      printf("   Link to %d from %s port %d SOCKET\n",
            g_net_link[i].pipe_node0, g_net_link[i].server_domain, g_net_link[i].server_port);
    }
  }

  fclose(fp);
  return (1);
}
