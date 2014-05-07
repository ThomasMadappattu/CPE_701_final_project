#ifndef _NETL_H_
#define _NETL_H_


/* NETWORK LAYER CONSTANTS */


#define NETWORK_LAYER_HEADER_SIZE 64
#define NETWORK_LAYER_SRC_OFFSET  0 
#define NETWORK_LAYER_SRC_SIZE    4 
#define NETWORK_LAYER_DEST_OFFSET 4
#define NETWORK_LAYER_DEST_SIZE   4
#define TYPE_OFFSET               8
#define TYPE_SIZE                 4 
#define ROUTE_OFFSET              12                 




#define DEST_NODE   0
#define NEXT_HOP    1 
#define LEN_IDX     2   



#define SRC_IDX                    1
#define DEST_IDX                   2
#define DISTANCE_IDX               3


#define ROUTE_TYPE                 4          

#define DIST_INF                   1000                 


#define COST(entry)                ( (entry) %  DIST_INF ) 
#define NODE(entry)                ( (entry) / DIST_INF )  

#define ROUTE_WAIT                 1

extern  void send_packet_link(void *buffer, int buf_len);
extern  void handle_network_layer_packet(void *buffer , int len); 
extern  void handle_packet_transport(void *buffer, int buf_len,int  dest_id);
extern  void init_network_layer();


#endif 
