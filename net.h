#define _GNU_SOURCE
#define MAX_FILE_NAME 100
#define PIPE_READ 0
#define PIPE_WRITE 1
enum bool {FALSE, TRUE};

/*
 * Struct used to store a link. It is used when the
 * network configuration file is loaded.
 */

struct net_link {
   enum NetLinkType type;
   int pipe_node0;
   int pipe_node1;
   int send_port;
   int server_port;
   char send_domain[MAX_FILE_NAME];
   char server_domain[MAX_FILE_NAME];
};


int net_init();

struct man_port_at_man *net_get_man_ports_at_man_list();
struct man_port_at_host *net_get_host_port(int host_id);

struct net_node *net_get_node_list();
struct net_port *net_get_port_list(int host_id);

struct net_data {
   int send_port;
   int server_port;
   int server_pipe;
   char send_domain[MAX_FILE_NAME];
   char server_domain[MAX_FILE_NAME];
   int switch_host_id;
};

struct net_data** get_g_net_data();
