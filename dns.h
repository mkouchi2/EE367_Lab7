#include "stdio.h"
#include <string.h>

#define DNS_SERVER_PHYS_ID 100
#define TABLE_SIZE 20
#define MAX_NAME_LENGTH 50

#define PING_BY_ID 1
#define PING_BY_NAME 2



struct dns_entry {
   char domain_name[MAX_NAME_LENGTH];
   int physical_id;
   int valid;
 };

void init_dns_table(struct dns_entry * naming_table);
void print_dns_table(struct dns_entry * naming_table);

