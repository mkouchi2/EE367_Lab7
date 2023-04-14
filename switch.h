#define MAX_TABLE_SIZE 100

void switch_main(int);

struct switch_job {
   struct packet *packet;
   int in_port_index;
   int out_port_index;
   struct switch_job *next;
};

struct switch_job_queue {
   struct switch_job *head;
   struct switch_job *tail;
   int occ;
};

void display_port_info(struct net_port*);

struct forward_table {
   int size;
   int valid[100];
   int HostID[100];
   int port[100];
};
