void print_job_queue_contents(struct job_queue *queue);
void display_packet_info(struct packet *pkt);
void display_job_number(struct job_queue *j);
const char* get_job_type_string(int job_type);
int job_q_num(struct job_queue *j_q);
void job_q_init(struct job_queue *j_q);
struct host_job *job_q_remove(struct job_queue *j_q);
void job_q_add(struct job_queue *j_q, struct host_job *j);
void reply_display_host_state(
      struct man_port_at_host *port,
      char dir[],
      int dir_valid,
      int host_id);
int get_man_command(struct man_port_at_host *port, char msg[], char *c);
int file_buf_remove(struct file_buf *f, char string[], int length);
int file_buf_add(struct file_buf *f, char string[], int length);
void file_buf_put_name(struct file_buf *f, char name[], int length);
void file_buf_get_name(struct file_buf *f, char name[]);
void file_buf_init(struct file_buf *f);

void display_host_job_info(struct host_job *job, int hostid);
