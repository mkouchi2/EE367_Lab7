/*
   This file contains the functions neccesary for the dns server. I will move them into the right files after writing them. 
*/

#include "dns.h"

void init_dns_table(struct dns_entry * naming_table) {
   for(int i = 0; i < TABLE_SIZE; i++) {
      naming_table[i].domain_name[0] = '\0';   // empty string
      naming_table[i].physical_id = -1;        // no id
      naming_table[i].valid = 0;               // not available    
   }
}

void print_dns_table (struct dns_entry * naming_table) {
   int i;
   for (i = 0; i < TABLE_SIZE; i++) {
      printf("Domain Name: %s\n", naming_table[i].domain_name);
      printf("Physical ID: %d\n", naming_table[i].physical_id);
      printf("Valid: %d\n", naming_table[i].valid);
   }
}



/*
 In the main file we can create a struct dns_entry dns_server[TABLE_SIZE]. Then pass &dns_server into the init_dns_server function
 */
