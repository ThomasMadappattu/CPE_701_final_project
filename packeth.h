#ifndef __PACKET_H_
#define __PACKET_H_

#include <stdio.h>
#include <stdlib.h> 
#include <string.h>



extern void pack_into_packet( void * packet_buffer , int offset, void *data, int data_size); 
extern void* get_packet_item( void * packet_buffer , int offset , int item_size); 
extern void *expand_packet( void **packet_buff , int buf_len ,int packet_size);
extern void *duplicate_packet(void *src_packet , void **dest_packet, int dup_len); 

#endif
