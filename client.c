#include "sockets.h"
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define PAYLOAD_MAX 1024

struct packet {
    char src;
    char dst;
    char type;
    int length;
    char payload[PAYLOAD_MAX];
};

void test_send_packet() {
    struct packet *p = malloc(sizeof(struct packet));
    p->src = 3;
    p->dst = 0;
    p->type = 1;
    p->length = 12;
    strncpy(p->payload, "Hello world", p->length);

    create_client("wiliki.eng.hawaii.edu", 3502, p);

    printf("Packet sent.\n");
}

int main() {
   
   test_send_packet(); 
   return 0;
}
