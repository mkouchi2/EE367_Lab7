/* 
 * host.h 
 */

enum host_job_type {
	JOB_SEND_PKT_ALL_PORTS,
	JOB_PING_SEND_REQ,	
	JOB_PING_SEND_REPLY,
	JOB_PING_WAIT_FOR_REPLY,
	JOB_FILE_DOWNLOAD_SEND,
   JOB_FILE_DOWNLOAD_RECV,
   JOB_FILE_UPLOAD_SEND,
	JOB_FILE_UPLOAD_RECV_START,
	JOB_FILE_UPLOAD_RECV_CONT,
   JOB_FILE_UPLOAD_RECV_END
};

struct host_job {
	enum host_job_type type;
	struct packet *packet;
	int in_port_index;
	int out_port_index;
	char fname_download[100];
	char fname_upload[100];
	int ping_timer;
	int file_upload_dst;
	int file_download_dst;
   struct host_job *next;
};


struct job_queue {
	struct host_job *head;
	struct host_job *tail;
	int occ;
};

void host_main(int host_id);
void display_packet_info(struct packet *);
void display_host_job_info(struct host_job*, int);
