#ifndef _LINKL_H_
#define _LINKL_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>



#define LINK_LAYER_RECV_SIZE_MAX 8096


#define MAX_NODES 91               
/* Link layer frame constants */ 

#define LINK_LAYER_HEADER_SIZE  128 

#define LINK_PROF_OFFSET         0 
#define LINK_PROTF_SIZE          4
#define LINK_CONTROL_OFFSET      (LINK_PROF_OFFSET + LINK_PROTF_SIZE)        
#define LINK_CONTROL_SIZE         8
#define LINK_EXTRA_OFFSET         (LINK_CONTROL_OFFSET + LINK_CONTROL_SIZE) 
#define LINK_EXTRA_INFO           8 
#define LINK_CKSUM_OFFSET         20 
#define LINK_CKSUM_SIZE           4         


struct link_inf
{
      int max_connections; 
      int current_nodes[MAX_NODES]; 
      int node_count;      


};

typedef struct link_inf LINK_INFO_T; 
extern LINK_INFO_T link_cfg; 
extern int is_current_node( int node_id );
extern void *server_thread(void *arg);
extern void link_send_buffer(void  *buffer , int len , int node_id);
extern void send_packet_to_network_layer(void *packet,int len);
extern int __loss_frac; /* packet loss fraction (between 0 and 100) */
extern int __corr_fact; /* packet corruption fraction (between 0 and 100) */




#endif







