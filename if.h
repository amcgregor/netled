//
// File        : if.h
// Description : The header file for if.c - and any program using the IF tools.
// Version     : 1.0
// Author      : Morgan Hughes (kyhm@kyhm.com)
// Maintainer  : Alice Bevan-McGregor (alice@gothcandy.com)
//

#ifndef HAVE_IF_THROUGHPUT_H
#define HAVE_IF_THROUGHPUT_H

typedef struct tag_if_throughput {
  char       if_name[16];
  long long  tot_bytes_in,  
             tot_bytes_out,
             chg_bytes_in,
             chg_bytes_out,
             time_ms,
             last_ms,
             sec_bytes_in,
             sec_bytes_out;
} if_throughput_t;

int get_if_throughput (if_throughput_t *ifp);
int is_interface_valid (char *interface);

#endif