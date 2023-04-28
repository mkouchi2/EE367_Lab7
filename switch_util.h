void display_port_info(struct net_port *p);
void display_forward_table(struct forward_table table);
void init_forward_table(struct forward_table *table);
void add_src_to_table(struct forward_table *table, struct packet *pkt, int port_index);
void send_to_all_ports(int node_port_num, struct net_port **node_port, struct packet *pkt);
int is_host_in_table(struct forward_table *table, char dst);
