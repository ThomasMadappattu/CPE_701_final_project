#ifndef __TRANSPORTL_H_
#define __TRANSPORTL_H_


#include "stateh.h"

#define TRASPORT_LAYER_SIZE 128 
#define SRC_PORT_OFFSET     0
#define SRC_PORT_SIZE       4 
#define DEST_PORT_OFFSET    4
#define DEST_PORT_SIZE      4
#define WIND_OFFSET         8
#define WIND_SIZE           4 
#define CONTROL_INFO_OFFSET        12 
#define CONTROL_INFO_SIZE          4
#define SEQ_NO_SIZE         4
#define SEQ_NO_OFFSET       16 
#define DATA_LEN_OFFSET     20 
#define DATA_LEN_SIZE       4        
#define CID_OFFSET        24
#define CID_SIZE           4
#define SENDER_ID_OFFSET   28
#define SENDER_ID_SIZE     4   
#define CRC_OFFSET         32
#define CRC_SIZE           4   



#define CONTROL_CONNECT 1 
#define CONTROL_ACK     2
#define CONTROL_CLOSE   3
#define CONTROL_DATA    4
#define CONTROL_CONNECT_ACK     5 
#define CONTROL_FIN      6

#define DUMMY_SIZE      32



#define MAX_CONNECTIONS 256  




#define STATE_CLOSE     -10 
#define STATE_CONNECTED  10
#define STATE_FIN        11    




#define WAIT_TIME_ACK     (500 * 1000)
#define RETRY_COUNT        5  




struct cid_hash_value
{
    void *data_buf ;
    STATE_T curr_state;  
    int buf_len; 
    int sequence_number; 
    int node_id; 
};

typedef struct cid_hash_value CID_HASH_VAL_T; 

struct cid_hast
{
    CID_HASH_VAL_T  cid_info [MAX_CONNECTIONS];

};

typedef struct cid_hast CID_TABLE; 
typedef struct cid_hast* CID_TABLE_P; 


struct snd_buf
{
     int response_code;
     int seq_no;
     int connection_id;  
     int mtu_possible;  

};

typedef struct snd_buf SEND_BUFFER; 




extern void init_transport_layer(); 
extern void handle_transport_layer_packet( void * buffer, int src_id , int dest_id);
extern void send_to_network_layer( void *buffer , int bufflen);
extern void handle_application_packet( void * buffer , int len , int dest_id);


extern STATE_T   __max_connections; 
extern int  allocate_sender();
extern void  deallocate_sender();


#endif 
