void create_server(int port, int pipe_fd);
void send_packet(int pipe_fd, struct packet *p);
void create_client(char* domain_name, int port, struct packet* p);
int receive_packet(int pipe_fd, struct packet *p);

