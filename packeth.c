/*
 * ******************************************* 
 *    Function: packeth.c 
 *
 *    Description : Packet helper routines 
 *
 *
 * ******************************************* 
 */

#include "packeth.h"

/*
 * ******************************************* 
 *   Function: pack_into_packet 
 *
 *   Description : packs an item into buffer
 *
 * ******************************************* 
 */
void pack_into_packet( void * packet_buffer , int offset, void *data, int data_size)
{
       
     char *dest_loc = (char *)packet_buffer + offset; 
     memcpy(dest_loc,data,data_size);   


}
/*
 * ******************************************* 
 *   Function: get_packet_item 
 *
 *   Description : gets an item from buffer
 *
 * ******************************************* 
 */

void* get_packet_item( void * packet_buffer , int offset , int item_size)
{

     void *item = malloc(item_size); 
     char *src_loc  = (char *)packet_buffer +  offset;  
     memcpy(item, src_loc , item_size);
     return item;   

}

/*
 * ******************************************* 
 *   Function: expand_packet  
 *
 *   Description : Expands a packet 
 *   eg expand a small data packec to network
 *   layer buffer and so on 
 *
 * ******************************************* 
 */


void *expand_packet( void **packet_buff , int buf_len ,int packet_size)
{
   
       void *tmp_buf = malloc( buf_len + packet_size) ;        
       memcpy((char *)tmp_buf + packet_size , *packet_buff, buf_len);
       return tmp_buf; 

}


/*
 * ******************************************* 
 *   Function: duplicate_packet 
 *
 *   Description : duplicates a packet  
 *
 * ******************************************* 
 */


void *duplicate_packet(void *src_packet , void **dest_packet, int dup_len)
{
    *dest_packet = malloc(dup_len);
    memcpy(*dest_packet , src_packet, dup_len);        
    return NULL;
}


#ifdef TEST
int main()
{
        char buffer[1024];
        int test_num = 45; 
        int *item; 
        /* packing test  num into buffer */
        pack_into_packet(buffer, 25,(void *)&test_num, sizeof(test_num));
        /* retriving item from the buffer */       
        item=(int *)get_packet_item(buffer ,25,sizeof(test_num));
        /* the item is  */   
        printf("\n The item is  = %d " , *item); 
        return  0; 

}

#endif 


