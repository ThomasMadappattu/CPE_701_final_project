/*
 * ******************************************* 
 *  File : netl.c 
 *
 *  Description: 
 *       network layer implmentation 
 *
 *
 *  Authors :  
 *        Sandeep Mathew & Mahmadur Khan   
 *
 * ******************************************* 
 *
 */



#include "netl.h"
#include "linkl.h"
#include "packeth.h"
#include "transportl.h" 
#include "cfparser.h"





/*
 *   Globals : 
 *              Routing tables , routing table lock 
 *
 *
 *
 */


int route_table[MAX_CONNECTIONS* 2]; 
int __total_entries = 0 ;
sem_t route_lock; 


void* send_route_packets( void* data);


/*
 * ******************************************* 
 *  Function: handle_network_layer_packet 
 *  
 *  Description : hadle a network layer packet
 *
 *  Parameters: 
 *     buffer - network layer buffer 
 *     len    - length of the buffer 
 *
 *
 *
 ******************************************** 
 */


/*
 * *******************************************
 *  Function: dump_route_table 
 *  Description:   
 *        dumps the routing table    
 *
 *
 *
 *
 * ******************************************* 
 */
void dump_route_table()
{
   FILE *fp;
   int count = 0 ; 
   fp = fopen("route_table.txt","w"); 
   for ( count  = 0 ; count  < MAX_CONNECTIONS + 1 ; count++)
   {

      fprintf(fp,"\n destination = %d  cost = %d" , NODE(route_table[count]) , COST(route_table[count])); 

   }
   fclose(fp);




}
/*
 * ******************************************* 
 *  Function: link_up
 *  Description : makes a link up 
 *  Parameters : 
 *     node - node to make up 
 *
 * ******************************************* 
 */
void link_up ( int  node )
/*
 * ******************************************* 
 *  Function: handle_network_layer_packet 
 *  
 *  Description : hadle a network layer packet
 *
 *  Parameters: 
 *     buffer - network layer buffer 
 *     len    - length of the buffer 
 *
 *
 *
 ******************************************** 
 */


{
     sem_wait(&route_lock);
/*
 * ******************************************* 
 *  Function: handle_network_layer_packet 
 *  
 *  Description : hadle a network layer packet
 *
 *  Parameters: 
 *     buffer - network layer buffer 
 *     len    - length of the buffer 
 *
 *
 *
 ******************************************** 
 */


     route_table[node] = 1 ; 
     sem_post(&route_lock); 

}
/*
 * ******************************************* 
 *  Function: link_down
 *  Description : makes a link down 
 *  Parameters : 
 *     node - node to make down  
 *
 * ******************************************* 
 */

void link_down( int node) 
{

      sem_wait(&route_lock);
      route_table[node]= DIST_INF -1;
      sem_post(&route_lock); 

}

/* 
 * *******************************************
 *   Function: add_route_entry 
 *
 *   Description: 
 *      Adds a routing table entry 
 *
 *   Parameters:
 *
 *       dest - destination 
 *
 *       next_hop - next_hop
 *
 *       cost - cost 
 *
 *
 *
 * ******************************************* 
 */

void add_route_entry( int dest , int next_hop , int cost)
{
       route_table[dest] = next_hop * DIST_INF + cost ; 
}

/*
 * ******************************************* 
 *  Function: init_network_layer  
 *
 *  Description : intializes network layer. 
 *  Set all items to a known state. 
 *
 *
 *
 * ******************************************* 
 */
void init_network_layer()
{
   
    int count = 0; 
    int node_index;
    pthread_t thread;  
    sem_init(&route_lock , 0 , 1 ) ; 
    node_index = find_node_by_id(__current_node);
    for ( count = 0 ; count < MAX_CONNECTIONS ; count++ )
    {
           route_table[count] =  DIST_INF-1 ; 
    
    } 
    for ( count  = 0 ; count < cfg_items[node_index].total_neighbours ; count++)
    {
            add_route_entry(cfg_items[node_index].neighbours[count],cfg_items[node_index].neighbours[count],1);

    }  
    pthread_create(&thread , NULL , send_route_packets , NULL);
}

/*
 * ******************************************* 
 *  Function: get_net_packet_type 
 *    
 *  Description : get the network packet type 
 *
 *  Parameters : 
 *       buffer - network layer packet  
 * ******************************************* 
 */

int get_net_packet_type( void *buffer) 
{
   int *pack_type = (int*) get_packet_item ( buffer , TYPE_OFFSET  , TYPE_SIZE); 
   return *pack_type; 


}
/*
 * ******************************************* 
 *  Function: send_route_packets 
 *
 *  Description : send route packets at 
 *  regular intervals ( this is a thread)  
 *  started when network layer is intialized 
 *
 * ******************************************* 
 */

void* send_route_packets( void* data)
{
    int count = 0; 
    int node_index;
    int pack_type = ROUTE_TYPE ;  
    int buff_len =  NETWORK_LAYER_HEADER_SIZE + MAX_CONNECTIONS * 2 * sizeof(int); 
    char *buffer = malloc(buff_len); 
    pack_into_packet(buffer, NETWORK_LAYER_SRC_OFFSET ,&__current_node,NETWORK_LAYER_SRC_SIZE); 
    pack_into_packet(buffer, TYPE_OFFSET , &pack_type , TYPE_SIZE ); 
    pack_into_packet(buffer, ROUTE_OFFSET , route_table ,  MAX_CONNECTIONS * 2 * sizeof(int));  
    sleep( ROUTE_WAIT * 30); 
    while ( TRUE ) 
    {
        sem_wait(&route_lock);
        node_index = find_node_by_id(__current_node);
        for ( count  = 0 ; count < cfg_items[node_index].total_neighbours ; count++)
        {
	    pack_into_packet(buffer, NETWORK_LAYER_DEST_OFFSET,&cfg_items[node_index].neighbours[count] ,NETWORK_LAYER_SRC_SIZE);
            sleep(ROUTE_WAIT);
	    link_send_buffer(buffer,buff_len + NETWORK_LAYER_HEADER_SIZE ,cfg_items[node_index].neighbours[count]);
        }  
        sleep(ROUTE_WAIT * 3);   
        sem_post(&route_lock); 
    }
    return NULL; 


}

/*
 * ******************************************* 
 *   Function :  handle_route_packets 
 *
 *   Description : 
 *        handle routing packets and update 
 *        routing tables  
 *
 *   
 *   Parameters: 
 *         buffer - routing table buffer 
 *         recevied      
 *
 * ******************************************* 
 */
void* handle_route_packets(void *buffer)
{

       int count = 0 ;  	  
       int *route_ptr; 
       int *src_id;  
       src_id  = (int *)get_packet_item(buffer ,NETWORK_LAYER_SRC_OFFSET  , NETWORK_LAYER_SRC_SIZE) ;   
       route_ptr = get_packet_item(buffer,ROUTE_OFFSET,2*sizeof(int) * MAX_CONNECTIONS ); 
       sem_wait(&route_lock);
       for ( count  = 0 ; count < MAX_CONNECTIONS ; count++)
       {
                if(  COST(route_ptr[count])  <  COST(route_table[count])   ) 
		{
                        route_table[count] =(*src_id) * DIST_INF  + COST(route_ptr[count]) + 1; 

		}
		else if ( NODE(route_ptr[count]) == *src_id)
		{
                        route_table[count] =(*src_id) * DIST_INF  + DIST_INF; 

		}

       }
       sem_post(&route_lock);        
       return NULL;    

}

/*
 * ******************************************* 
 *  Function: handle_network_layer_packet 
 *  
 *  Description : hadle a network layer packet
 *
 *  Parameters: 
 *     buffer - network layer buffer 
 *     len    - length of the buffer 
 *
 *
 *
 ******************************************** 
 */

void handle_network_layer_packet(void *buffer,int len)
{

   
   int *dest_id;
   int *src_id; 
   char *transport_start;   
   pthread_t thread;
   int count = 0 ;
   /*
        extract destination , see if destination
        is current address

        Also handle segmentaion and reassembly here         

    */
   dest_id = (int *)get_packet_item(buffer ,NETWORK_LAYER_DEST_OFFSET , NETWORK_LAYER_DEST_SIZE ) ;
   src_id  = (int *)get_packet_item(buffer ,NETWORK_LAYER_SRC_OFFSET  , NETWORK_LAYER_SRC_SIZE) ;   
   printf("\n Soource id = %d Destination id = %d" , *src_id , *dest_id);
 
   if ( get_net_packet_type(buffer) == ROUTE_TYPE ) 
   {
  
          pthread_create(&thread,NULL , handle_route_packets , buffer);
  
   } 
   else if ( is_current_node(*dest_id))
   {
            /* is the current node the destination */ 
            /* send the packet to transport_layer */ 
            transport_start = (char *)buffer  + NETWORK_LAYER_HEADER_SIZE; 
  	    handle_transport_layer_packet(transport_start,*src_id , *dest_id); 
                     
   }
   else
   {

         /*
             Add routing code here 
         */
         if ( COST(route_table[*dest_id])  <  ( DIST_INF - 1) )
	 {
             link_send_buffer(buffer,len,NODE(route_table[*dest_id]));       

	 }	 
	 else
	 {
                /*Choose the first router avialable as the default router */
		for ( count  = 0 ; count < MAX_CONNECTIONS +1 ; count++)
		{
                         if ( COST(route_table[count]) < (  DIST_INF - 1)  )
					 break; 
		
		}	

	        link_send_buffer(buffer,len,NODE(route_table[count]));     
	 }
   
   }     


}

/*
 * ******************************************* 
 *  Function: handle_packet_transport 
 *  
 *  Description : hadle a transport layer packet
 *
 *  Parameters: 
 *     buffer - network layer buffer 
 *     len    - length of the buffer 
 *     dest_id - destination id  
 *
 *
 ******************************************** 
 */


void handle_packet_transport(void *buffer, int buf_len,int  dest_id)
{
   
    int count; 
    buffer = expand_packet(&buffer , buf_len , NETWORK_LAYER_HEADER_SIZE); 
    pack_into_packet(buffer, NETWORK_LAYER_SRC_OFFSET ,&__current_node,NETWORK_LAYER_SRC_SIZE); 
    pack_into_packet(buffer, NETWORK_LAYER_DEST_OFFSET, &dest_id,NETWORK_LAYER_SRC_SIZE);
      /*
             Add routing code here 
         */
         if ( COST(route_table[dest_id])  <  (DIST_INF-1) )
	 {
             link_send_buffer(buffer,buf_len + NETWORK_LAYER_HEADER_SIZE, NODE(route_table[dest_id]));       

	 }	 
	 else
	 {
                /*Choose the first router avialable as the default router */
		for ( count  = 0 ; count < MAX_CONNECTIONS +1 ; count++)
		{
                         if ( COST(route_table[count]) < (DIST_INF-1) )
					 break; 
		
		}	

	        link_send_buffer(buffer,buf_len + NETWORK_LAYER_HEADER_SIZE,NODE(route_table[count]));     
	 }



}

