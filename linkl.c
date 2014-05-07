/*
 *  File:  linkl.c 
 *
 *   Description : link layer implementation 
 *
 *
 *    Autors : Sandeep Mathew & Mahamadur Khan 
 *
 *
 *
 *
 */





#include "cfparser.h" 
#include "linkl.h" 
#include "netdb.h" 
#include <strings.h>
#include <string.h>
#include <stdint.h>
#include "netl.h"
#include "packeth.h"
#include <mhash.h> 
#include <math.h>


#define DEBUG

LINK_INFO_T link_cfg;




/*
 * ******************************************* 
 *  Function: get_crc 
 *
 *  Description: gets the crc of a packet   
 *   
 *  Parameters : 
 *     buffer - link layer buffer 
 *     len    - length of buffer 
 *  
 * ******************************************* 
 */
uint32_t get_crc(void *buffer , int len)
{
     
	MHASH td;
	uint32_t crc;
	td = mhash_init(MHASH_CRC32);
        mhash(td,buffer,len); 
        mhash_deinit(td,&crc);
	return crc; 
 

}


/*
 * ******************************************* 
 *  Function: sendto_garbled 
 *
 *  Description : send to garbled provided 
 *   by Dr Yuksel    
 *
 *
 *  
 * ******************************************* 
 */


int __loss_frac = 0; /* packet loss fraction (between 0 and 100) */
int __corr_fact = 0; /* packet corruption fraction (between 0 and 100)*/

/* Wrapper function to emulate an erroneous link */
int sendto_garbled(int s, const void *msg, size_t len, int flags, 
		   const struct sockaddr *to, socklen_t tolen)
{
  int localint, i;
  char * localpoint;

  srand48(11);/* initialize the random number generator*/

  /* Decide whether to loose this packet */
  if (drand48()*100 <  __loss_frac)
    return len; /* act as if the packet was sent correctly!*/

  /* Decide whether to corrupt this packet */
  if (drand48()*100 <__corr_fact)
    if (len > 0) {
      /* pick a random location to corrupt*/
      localint = round(drand48()*len);
      i = 0;
      localpoint = (char *)msg;
      while (i < localint) {
	i++;
	localpoint++;
      }
      *localpoint = ~(*localpoint);
    }

  return sendto(s, msg, len, flags, (struct sockaddr *)to, tolen);
}

/*
 * ******************************************* 
 *  Function: link_send_buffer  
 *
 *  Description: 
 *         sends a link buffer using udp
 *         sockets afer adding checksum  
 *   
 *  Parameters: 
 *          buffer - link layer buffer 
 *          node_id - node id 
 *
 * ******************************************* 
 */


void link_send_buffer(void  *buffer , int len , int node_id)
{
     


     int sock_des; 
     struct sockaddr_in server; 
     struct hostent *hp;  
     int node_index;
     uint32_t crc;
     void *snd_buffer;  
     
      
     server.sin_family = AF_INET;
     snd_buffer = expand_packet(&buffer , len  , LINK_LAYER_HEADER_SIZE);
     crc = get_crc((char *) snd_buffer + LINK_LAYER_HEADER_SIZE  , len);  
     pack_into_packet(snd_buffer , LINK_CKSUM_OFFSET , &crc , LINK_CKSUM_SIZE);    
     node_index = find_node_by_id(node_id);
     if ( node_index != NOT_FOUND)
     {
 
           hp = gethostbyname(cfg_items[node_index].node_name);
           bcopy(hp->h_addr,&(server.sin_addr.s_addr),hp->h_length);      
           server.sin_port = htons(cfg_items[node_index].udp_port);
     }
     sock_des = socket(AF_INET , SOCK_DGRAM , 0); 
     bind(sock_des,( struct sockaddr *) &server , sizeof(server));
     sendto_garbled( sock_des , snd_buffer , len + LINK_LAYER_HEADER_SIZE,0,(struct sockaddr *)&server, sizeof(server));
     
               
}

/*
 * ******************************************* 
 *   Function: is_current_node 
 *
 *   Description : check if given node id 
 *    is the current node 
 *
 *   Parameters : 
 *      node_id - node id to check
 *
 * ******************************************* 
 */
int is_current_node( int node_id )
{
     
   return (node_id == 	 __current_node  );  
}

/*
 * ******************************************* 
 *  Function: start_link_server  
 *
 *  Description : starts the link server 
 *  and accpets packets using upd sockets  
 *  this is a thread   
 *
 *  Parameters:
 *     node_id - current node' s id    
 *
 * ******************************************* 
 */

void start_link_server(int node_id)
{
     int sock_des; 
     struct sockaddr_in server; 
     char buffer[LINK_LAYER_RECV_SIZE_MAX];       
     int recv_size;
     uint32_t *rec_crc;
     uint32_t packet_crc; 
     struct hostent *hp;  
     int node_index; 
     server.sin_family = AF_INET;
     node_index = find_node_by_id(node_id);
       
     if ( node_index != NOT_FOUND)
     {
 
           hp = gethostbyname(cfg_items[node_index].node_name);
           bcopy(hp->h_addr,&(server.sin_addr.s_addr),hp->h_length);      
     
           #ifdef DEBUG  

              printf(" node_index = %d " , node_index);
              printf(" port  = %d" ,(cfg_items[node_index].udp_port)); 
           #endif   
           server.sin_port = htons(cfg_items[node_index].udp_port);
     }
     else 
     {
          #ifdef DEBUG   
             printf("\n Node id not found = %d \n" , node_id);
	  #endif

     }
     sock_des = socket(AF_INET , SOCK_DGRAM , 0); 
     if ( bind(sock_des,( struct sockaddr *) &server , sizeof(server)) < 0 ) 
     {
          perror("Bind failed .. .") ;
          printf("\n bind failed  ... ");   
     }
     while (TRUE)
     {
             recv_size = recv ( sock_des , buffer , sizeof(buffer) , 0 );
             packet_crc =  get_crc((char *)buffer + LINK_LAYER_HEADER_SIZE ,recv_size - LINK_LAYER_HEADER_SIZE);
	     rec_crc   = (uint32_t *)  get_packet_item(buffer , LINK_CKSUM_OFFSET , LINK_CKSUM_SIZE ); 
             if ( *rec_crc == packet_crc ) 
	     {

                  send_packet_to_network_layer(buffer,recv_size); 
	     }
             else 
	     {
                   fprintf(stderr ,"\n CRC check failed , packet dropped !"); 
                   fprintf(stderr , "Rec length  = %d " ,recv_size );  
                   fprintf(stderr , " Packet crc = %u  rec crc = %u " , packet_crc , *rec_crc); 
	
	     } 	     
     }    

 
}

/*
 * ******************************************* 
 *  Function: send_packet_to_network_layer 
 *
 *  Description : sends a received link layer
 *  packet to network layer 
 *
 *  Parameters : 
 *     packet - the link layer buffer 
 *
 *     len - length of the packet 
 *
 *
 * ******************************************* 
 */
void send_packet_to_network_layer(void *packet, int len)
{
    /* Take off the link layer header and give to network_layer */       
    char *network_start; 
         
    network_start = (char *)packet  + LINK_LAYER_HEADER_SIZE ; 
    /* route the packet to network layer */ 
    handle_network_layer_packet(network_start,len-LINK_LAYER_HEADER_SIZE);                  

}


/* 
 * Function: server_thread 
 *
 * Description: start the server thread 
 *
 *
 */

void *server_thread(void *arg)
{
   int *node_id = (int *)arg;
   printf("\n node id = %d" , *node_id);
   start_link_server(*node_id);  
   return NULL; 

}




#ifdef TEST


#include <pthread.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h> 

void* test_sender_thread(void *arg)
{

   int *node_id = (int *)arg;
   int sockdes;
   while(TRUE)
   {
        sleep(1); 
        link_send_buffer("Hello World",5 ,*node_id );    

   }  
   

}


int main ()
{

  pthread_t thread;
  pthread_t cthread; 
  int node_id = 1;
  read_config_file("test.txt");   
  pthread_create(&thread,NULL ,server_thread, &node_id);
  pthread_create(&cthread,NULL ,test_sender_thread, &node_id);
  pthread_join(thread,NULL);
  pthread_join(cthread,NULL);
  return 0 ;    
  


}


#endif 

