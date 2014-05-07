#ifndef __NFDAPP_H_
#define __NFDAPP_H_


#include "cfparser.h"


#define BUF_MAX 100 


#define MAX_NAME 100 

#ifndef NAME_MAX    
  #define NAME_MAX 100
#endif 

#define CODE_OFFSET  0 
#define CODE_SIZE    4


#define SIZE_OFFSET  4
#define SIZE_SIZE    4 


#define FILENAME_OFFSET  8
#define FILENAME_SIZE   100

#define SRC_ID_OFFSET   108 
#define SRC_ID_SIZE     4

#define DATA_OFFSET     112


#define APP_HEADER_SIZE   ( CODE_SIZE + FILENAME_SIZE + SIZE_SIZE  + SRC_ID_SIZE) 


#define CODE_SUCCESS  0 
#define CODE_FAILURE  1 
#define CODE_GET      2



struct snd_args  
{
    void *buffer; 
    int len; 
    int dest_id; 

}; 

typedef struct snd_args* SND_ARGS_T; 


extern void*  sender_thread(void *args); 
extern void handle_app_buffer(void *buffer , int cid); 
extern void send_buffer_transport(void *buffer , int len , int dest_id); 
extern void nfd_buffer_receved(void *buffer , int len);
extern void nfd_send_buffer_transport(void *buffer , int len, int dest_id);
extern void dump_route_table(); 
extern void link_up(int node);
extern void link_down(int node);
#endif 
