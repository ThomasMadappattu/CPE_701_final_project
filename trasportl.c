/*
 * *******************************************
 * File: transportl.c 
 * 
 * Description : 
 *        transport layer implementation     
 *
 *
 * Authors : Sandeep Mathew & Mahmadur Khan
 *
 * 
 * ******************************************* 
 */



#include "transportl.h"
#include "nfdapp.h"
#include "packeth.h"
#include "netl.h"
#include "cfparser.h"
#include "stateh.h"
#include <pthread.h>
#include <time.h> 
#include <mhash.h> 






/*
 *  Globals:
 *       Per session buffer , max conncetions possible ,
 *       connection id pools for send and receiver  
 *        
 *
 */

CID_TABLE __session_tab; 
STATE_T   __max_connections; 
SEND_BUFFER  sender_buffer[MAX_CONNECTIONS]; 
int sender_id = MAX_CONNECTIONS-1; 
sem_t sender_lock; 
sem_t connect_lock;

int connection_pool[MAX_CONNECTIONS]; 
int sender_pool[MAX_CONNECTIONS];


/*
 * ******************************************* 
 *  Function: init_transport_layer 
 *
 *  Description: 
 *      initializes the transport layer, 
 *      set up state for the global data
 *      items 
 *
 *   
 * *******************************************  
 *
 */

void init_transport_layer()
{
    int count = 0 ;
    init_network_layer(); 
    for ( count = 0 ; count < MAX_CONNECTIONS ; count++)
    {
         init_state(&__session_tab.cid_info[count].curr_state);
	 set_state(&__session_tab.cid_info[count].curr_state,STATE_CLOSE);
         connection_pool[count] = FALSE;
         sender_pool[count] = FALSE; 	 
             
    }
    sem_init(&sender_lock , 0 , 1); 
    sem_init(&connect_lock , 0 , 1); 

}

/*
 * ******************************************* 
 *  Function: min_of  
 *
 *  Description: 
 *       utility function to find minimum    
 *       of 2 values  
 * 
 *   Parameters : 
 *       val1 - Value1 
 *       val2 - Value2 
 *
 *
 * ******************************************* 
 */

int min_of ( int val1 , int val2)
{

   return ( (val1 < val2 ) ? val1 : val2); 
}

/*
 ******************************************** 
 * Function: allocate_sender 
 * Description: allocates a sender    
 *  
 * Returns: the sender id 
 *
 *
 ******************************************** 
 */
int  allocate_sender()
{
  
    int ret_val;	
    int count = 0; 
    sem_wait(&sender_lock);  
    while ( count < MAX_CONNECTIONS)
    {
	    if ( sender_pool[count] == FALSE) 
	    {
	      
	      sender_pool[count] = TRUE;	    	    	    
	      ret_val = count; 
	      break; 
	    }
            count++; 


    }
     
    sem_post(&sender_lock);
    return ret_val;

}

/*
 * *******************************************  
 * Function: deallocate_sender
 * Description: deallocates the sender 
 * Parameters: 
 *  sender_id - id of the sender to be deall 
 *  cated                            
 *
 *
 * ******************************************* 
 */

void  deallocate_sender( int sender_id )
{        
        sem_wait(&sender_lock);     
        sender_pool[sender_id] = FALSE;  
	sem_post(&sender_lock);
}
/*
 * *******************************************
 *  Function: allocate_connection 
 *
 *  Description: allocates a connection id  
 * 
 *  Parameters:
 *      max - maximum connections allowed 
 *
 *  Returns: the allocated conncetion id
 * *******************************************
 *
 */

int  allocate_connection(int max )
{
  
    int ret_val;	
    int count = 0; 
    sem_wait(&connect_lock);  
    while ( count < max)
    {
	    if ( connection_pool[count] == FALSE) 
	    {
	      
	      connection_pool[count] = TRUE;	    	    	    
	      ret_val = count; 
	      break; 
	    }
            count++; 


    }
     
    sem_post(&connect_lock);
    return ret_val;

}

/*
 * *******************************************
 *  Function: deallocate_connection 
 *  Description: deallocates a conncetion id
 *  Parameters : 
 *     connect_id - conenciton id  
 *   
 *
 * ******************************************* 
 */

void  deallocate_connection( int  connect_id )
{        
        sem_wait(&connect_lock);     
        connection_pool[connect_id] = FALSE;  
	sem_post(&connect_lock);
}

/*
 * *******************************************
 * Function: set_packet_cid 
 *
 * Description: 
 *   sets cid of a packet 
 *
 * Parameters : 
 *    packet - packet buffer 
 *    cid    - conneciton id  
 *
 *
 * ******************************************* 
*/

void set_packet_cid (void *packet ,int cid)
{
       pack_into_packet(packet,CID_OFFSET,&cid,CID_SIZE);      

}
/*
 * ******************************************* 
 *  Function: 
 *     send_connect_ack 
 *
 *  Description: 
 *     Sends connection acknoledgment packet 
 *
 *  Parameters: 
 *      dest_id  - destination 
 *
 *      req_mtu - requested mtu   
 *
 *      cid     - conncection id 
 *
 *
 *      sid     - service id 
 *
 *
 *
 * ******************************************* 
 */
void send_connect_ack( int dest_id  , int req_mtu , int cid , int sid  )
{
    char buffer[DUMMY_SIZE]; 
    int my_mtu; 
    int ctrl_cmd  = CONTROL_CONNECT_ACK; 
    void *buff  = (void *)buffer; 
    my_mtu = min_of (  cfg_items[__current_node-1].mtu_size - 3 * TRASPORT_LAYER_SIZE , req_mtu ) ; 
    buff = expand_packet(&buff,DUMMY_SIZE , TRASPORT_LAYER_SIZE ); 
    pack_into_packet(buff , CONTROL_INFO_OFFSET ,&ctrl_cmd, CONTROL_INFO_SIZE);
    pack_into_packet(buff , WIND_OFFSET ,&my_mtu , WIND_SIZE);    
    set_packet_cid(buff,cid);   
    pack_into_packet(buff , SENDER_ID_OFFSET , &sid , SENDER_ID_SIZE); 
    handle_packet_transport(buff , DUMMY_SIZE + TRASPORT_LAYER_SIZE ,dest_id); 
    



}


/*
 * ******************************************* 
 *  Function:  send_connect_request 
 *
 *  Description:
 *      sends connect request t
 *
 *  Parameters: 
 *      dest_id - Destination id 
 *
 *      sender_id - Sender id 
 * ******************************************* 
 * */

void send_connect_request(int dest_id , int sender_id )
{
    char buffer[DUMMY_SIZE]; 
    int my_mtu; 
    int ctrl_cmd  = CONTROL_CONNECT; 
    void *buff  = (void *)buffer; 
    my_mtu = cfg_items[__current_node-1].mtu_size - 3 * TRASPORT_LAYER_SIZE; 
    printf("\n Mtu =  %d" , cfg_items[__current_node-1].mtu_size ) ; 
    printf("\n My mtu  = %d \n " , my_mtu);
    buff = expand_packet(&buff,DUMMY_SIZE , TRASPORT_LAYER_SIZE ); 
    pack_into_packet(buff , CONTROL_INFO_OFFSET ,&ctrl_cmd, CONTROL_INFO_SIZE);
    pack_into_packet(buff , WIND_OFFSET ,&my_mtu , WIND_SIZE);     
    pack_into_packet(buff , SENDER_ID_OFFSET , &sender_id , SENDER_ID_SIZE); 
    handle_packet_transport(buff , DUMMY_SIZE + TRASPORT_LAYER_SIZE ,dest_id); 
    
 
}

/*
 * ******************************************* 
 *  Function: send_ack_request 
 *  
 *  Description: sends acknowledgmetn request  
 *
 *  Parameters : 
 *   dest_id - destination id 
 *   seq_no  - sequence number 
 *   sid  - sender  id 
 *
 *
 * ******************************************* 
 */
void send_ack_request(int dest_id , int seq_no  , int sid )
{
   
    char buffer[DUMMY_SIZE]; 
    void *buff  = (void *)buffer;
    int ctrl_cmd  = CONTROL_ACK; 

    buff = expand_packet(&buff,DUMMY_SIZE , TRASPORT_LAYER_SIZE ); 
    pack_into_packet(buff , CONTROL_INFO_OFFSET , &ctrl_cmd , CONTROL_INFO_SIZE); 
    pack_into_packet(buff , SEQ_NO_OFFSET , &seq_no , SEQ_NO_SIZE);
    pack_into_packet(buff , SENDER_ID_OFFSET , &sid , SENDER_ID_SIZE); 
    handle_packet_transport(buff , DUMMY_SIZE + TRASPORT_LAYER_SIZE ,dest_id); 
      

}
/*
 * ******************************************* 
 *  Function: send_fin_request  
 *  Description : sends the fin request
 *  Parameters : 
 *     dest_id - destination id 
 *     cid     - connection  id 
 *     sid     - sender id 
 *
 *  ******************************************* 
 *
 */
void send_fin_request(int dest_id  , int cid , int sid )
{
   
    char buffer[DUMMY_SIZE]; 
    void *buff  = (void *)buffer;
    int ctrl_cmd  = CONTROL_FIN; 

    buff = expand_packet(&buff,DUMMY_SIZE , TRASPORT_LAYER_SIZE ); 
    pack_into_packet(buff , CONTROL_INFO_OFFSET , &ctrl_cmd , CONTROL_INFO_SIZE); 
    set_packet_cid(buff,cid);   
    pack_into_packet(buff , SENDER_ID_OFFSET , &sid , SENDER_ID_SIZE); 
 
    handle_packet_transport(buff , DUMMY_SIZE + TRASPORT_LAYER_SIZE ,dest_id); 
      

}
/*
 * ******************************************* 
 *  Function: send_close_request 
 *
 *  Description: sends close request  
 *
 *  Parameters : 
 *       dest_id - destination id 
 *       cid     - connection id 
 *       sid     - sender id 
 *
 *
 * ******************************************* 
 *
 */
void send_close_request(int dest_id  , int cid  , int sid)
{
   
    char buffer[DUMMY_SIZE]; 
    void *buff  = (void *)buffer;
    int ctrl_cmd  = CONTROL_CLOSE; 

    buff = expand_packet(&buff,DUMMY_SIZE , TRASPORT_LAYER_SIZE ); 
    pack_into_packet(buff , CONTROL_INFO_OFFSET , &ctrl_cmd , CONTROL_INFO_SIZE); 
    set_packet_cid(buff,cid);   
    pack_into_packet(buff , SENDER_ID_OFFSET , &sid , SENDER_ID_SIZE); 
 

    
    handle_packet_transport(buff , DUMMY_SIZE + TRASPORT_LAYER_SIZE ,dest_id); 
      

}


/*
 * ******************************************* 
 *  Function: send_data_packet 
 *
 *  Description : 
 *      sends a data packet 
 *
 *  Parameters : 
 *       dest_id - destination id 
 *       seq_no  - sequence no 
 *       cid     - connection id 
 *       sid     - sender id 
 *       data    - data buffer 
 *       data_len - length of data buffer 
 *
 *
 * ******************************************* 
 */
void send_data_packet( int dest_id , int seq_no  ,  int cid , int sid , void *data , int data_len )
{	
    char *buffer[DUMMY_SIZE]; 
    void *buff  = (void *)buffer;
    int ctrl_cmd  = CONTROL_DATA; 

    buff = expand_packet(&buff,0, TRASPORT_LAYER_SIZE + data_len ); 
    pack_into_packet(buff , CONTROL_INFO_OFFSET , &ctrl_cmd , CONTROL_INFO_SIZE); 
    pack_into_packet(buff , SEQ_NO_OFFSET , &seq_no , SEQ_NO_SIZE);
    pack_into_packet(buff , TRASPORT_LAYER_SIZE , data , data_len); 
    pack_into_packet(buff , SENDER_ID_OFFSET , &sid , SENDER_ID_SIZE); 
    pack_into_packet(buff , WIND_OFFSET , &data_len , WIND_SIZE);
    set_packet_cid(buff , cid);
    handle_packet_transport(buff , data_len + TRASPORT_LAYER_SIZE ,dest_id);    


}	

/*
 * ******************************************* 
 *  Function: get_packet_type 
 *
 *  Description : gets the packet type 
 *
 *  Parameters : 
 *         packet - the current packet buffer
 *
 * ******************************************* 
 */
int get_packet_type (void *packet)
{

     int *control_cmd = (int *)get_packet_item(packet,CONTROL_INFO_OFFSET , CONTROL_INFO_SIZE );
     return *control_cmd;  

}

/*
 * ******************************************* 
 *  Function: get_requested_mtu 
 *
 *  Description : gets the requested mtu 
 *  from the packet 
 *
 *  Parameters: 
 *       packeth - the current packet buffer
 *
 * ******************************************* 
 */
int get_requested_mtu( void *packet)
{


     int *mtu = (int *)get_packet_item(packet,WIND_OFFSET , WIND_SIZE );
     return *mtu;  

}
/*
 * ******************************************* 
 *  Function: get_cid 
 *
 *  Description : gets the connection id  
 *  from the packet 
 *
 *  Parameters: 
 *       packeth - the current packet buffer
 *
 * ******************************************* 
 */

int get_cid( void *packet)
{

     int *cid = (int *)get_packet_item(packet,CID_OFFSET , CID_SIZE );
     return *cid;  
}
/*
 * ******************************************* 
 *  Function: get_sequence_number  
 *
 *  Description : gets the sequence number  
 *  from the packet 
 *
 *  Parameters: 
 *       packeth - the current packet buffer
 *
 * ******************************************* 
 */

int get_sequence_number( void *packet)
{

	int *seq_no = (int *)get_packet_item(packet , SEQ_NO_OFFSET , SEQ_NO_SIZE );
	return *seq_no; 

}
/*
 * ******************************************* 
 *  Function: get_window_size  
 *
 *  Description : gets the window size   
 *  from the packet 
 *
 *  Parameters: 
 *       packeth - the current packet buffer
 *
 * ******************************************* 
 */


int get_window_size( void *packet)
{

	int *window_size = (int *)get_packet_item(packet , WIND_OFFSET , WIND_SIZE );
	return *window_size; 

}
/*
 * ******************************************* 
 *  Function: get_sender_id  
 *
 *  Description : gets the sender id    
 *  from the packet 
 *
 *  Parameters: 
 *       packeth - the current packet buffer
 *
 * ******************************************* 
 */


int get_sender_id(void *packet)
{

	int *sender_id = (int *)get_packet_item(packet , SENDER_ID_OFFSET ,SENDER_ID_SIZE );
	return *sender_id; 

}


/*
 * ******************************************* 
 *  Function: handle_transport_layer_packet  
 *
 *  Description: 
 *
 *  handle a transport layer 
 *  packet. Core transport layer handling 
 *  function , all the other funcitons 
 *  were written to support this big guy 
 *  
 *  Parameters: 
 *     buffer - packet buffer 
 *     src_id  - Source id 
 *     dest_id - Destination id 
 *
 * ******************************************* 
 */
void handle_transport_layer_packet( void * buffer , int src_id , int dest_id )
{

      
        int packet_type; 
        int requested_mtu; 
        int seq_no; 
        int connect_id; 
        void *realloc_buffer;
        int win_size ;	
	void *packet_data;
        int cid_seq_no; 	
        int current_sender_id; 
	int current_cid_state; 
	int sender_id ;
        char *realloc_buf_ptr; 	
	int max_connections_permitted = get_state(&__max_connections);
	packet_type = get_packet_type(buffer);

	switch(packet_type)
	{
		case CONTROL_CONNECT:
			requested_mtu = get_requested_mtu(buffer);
		        connect_id  =allocate_sender(max_connections_permitted);
			sender_id  = get_sender_id(buffer);
		        if ( connect_id  <  max_connections_permitted)
			{
                             
		              init_state(&__session_tab.cid_info[connect_id].curr_state);
		              set_state(&__session_tab.cid_info[connect_id].curr_state ,STATE_CONNECTED);
		              __session_tab.cid_info[connect_id].sequence_number = 0;  
		              __session_tab.cid_info[connect_id].buf_len = 0;  	
			        
                              send_connect_ack(src_id  , requested_mtu , connect_id,sender_id); 

			}
		        else
		        {
                              fprintf(stderr ,"%s", "\n Connection not possilbe  all slots full \n"); 
                              
		        }	
                        break;
		case CONTROL_ACK:
                        seq_no = get_sequence_number(buffer);
                        current_sender_id = get_sender_id(buffer);
			connect_id  = get_cid(buffer); 
			sender_buffer[current_sender_id].seq_no = seq_no; 
		        sender_buffer[current_sender_id].response_code = CONTROL_ACK; 
			sender_buffer[current_sender_id].connection_id = connect_id; 
                        break;
		case CONTROL_DATA:
		   {	
                       
			fprintf(stderr,"DATA packet received  ... ");    
		        connect_id  = get_cid(buffer);
		        seq_no = get_sequence_number(buffer);
			sender_id = get_sender_id(buffer);
                        cid_seq_no = __session_tab.cid_info[connect_id].sequence_number;
                        current_cid_state = get_state(&__session_tab.cid_info[connect_id].curr_state); 			
			fprintf(stderr,"\n seq no = %d cid seq no = %d window size = %d  CID = %d" , seq_no , cid_seq_no , get_window_size(buffer) , connect_id);
		        /* are we getting the next sequnce number  & we are connected */
			if( (  seq_no == cid_seq_no + 1 ) )
			{
                              win_size = get_window_size(buffer); 
		              realloc_buffer = malloc(win_size + __session_tab.cid_info[connect_id].buf_len);
		              if ( __session_tab.cid_info[connect_id].buf_len >  0  && ( current_cid_state != STATE_CLOSE)) 
			      {
			              memcpy(realloc_buffer,__session_tab.cid_info[connect_id].data_buf,__session_tab.cid_info[connect_id].buf_len);
				      realloc_buf_ptr = (char *) realloc_buffer + __session_tab.cid_info[connect_id].buf_len ;
				      packet_data    = (char *) buffer + TRASPORT_LAYER_SIZE ;  
				      memcpy(realloc_buf_ptr,packet_data , win_size);
				      __session_tab.cid_info[connect_id].buf_len += win_size; 
				      free( __session_tab.cid_info[connect_id].data_buf); 
				      __session_tab.cid_info[connect_id].data_buf = realloc_buffer;
                                      __session_tab.cid_info[connect_id].sequence_number++;
				      send_ack_request(src_id,seq_no,sender_id) ;   
		              }	     
			      else
			      {

				      packet_data    = (char *) buffer + TRASPORT_LAYER_SIZE ;  
				      memcpy(realloc_buffer,packet_data , win_size); 
				      __session_tab.cid_info[connect_id].buf_len += win_size; 
				      __session_tab.cid_info[connect_id].data_buf = realloc_buffer;
                                      __session_tab.cid_info[connect_id].sequence_number++;
				      send_ack_request(src_id,seq_no,sender_id) ;  

			      } 
                             
			}
		        else  if (  seq_no == cid_seq_no   )
			{
			        send_ack_request(src_id,seq_no,sender_id) ;   

			
			}
			else
			{
                            fprintf(stderr , "%s" , "\n Packet out of sequence number / duplicate  discarded \n ");  

			}	
                      }     
		      break; 

		case CONTROL_FIN:
		      fprintf(stderr ,"\n fin received  "); 
                      connect_id  = get_cid(buffer);
		      sender_id   = get_sender_id(buffer);
		      current_cid_state = get_state(&__session_tab.cid_info[connect_id].curr_state); 			
                      if ( current_cid_state  == STATE_CONNECTED )
		      {  
                              nfd_buffer_receved(__session_tab.cid_info[connect_id].data_buf,__session_tab.cid_info[connect_id].buf_len);
		              free(__session_tab.cid_info[connect_id].data_buf);
		              __session_tab.cid_info[connect_id].buf_len = 0 ; 
 		      }
		      else
		      {

                               set_state(&__session_tab.cid_info[connect_id].curr_state,STATE_FIN); 	
		      }
		      send_ack_request(src_id,0,sender_id); 
		      break;

		case  CONTROL_CLOSE:
                      connect_id  = get_cid(buffer);
		      set_state(&__session_tab.cid_info[connect_id].curr_state,STATE_CLOSE);
                      deallocate_connection(connect_id);
		      fprintf(stderr , "\nRecevied close packet "); 
		      break;

		case CONTROL_CONNECT_ACK:
	               seq_no = get_sequence_number(buffer);
                       current_sender_id = get_sender_id(buffer);
		       connect_id  = get_cid(buffer); 
		       sender_buffer[current_sender_id].response_code = CONTROL_CONNECT_ACK;
		       sender_buffer[current_sender_id].mtu_possible = get_window_size(buffer);  
		       sender_buffer[current_sender_id].connection_id = connect_id; 
	               fprintf(stderr , "\n Recieved control connect ack  sender id %d  mtu_possible  %d " ,current_sender_id, sender_buffer[current_sender_id].mtu_possible );  
 	       
 	               break; 

                default:
		       /* None of the known control types */
		       /* dropping packet */
		      fprintf(stderr,"\n Not one of the known packet types, Type = %d " , packet_type ); 
		      break; 

	}

}

/*
 * ******************************************* 
 *  Function: send_data_packet_reliable 
 *
 *
 *  Description: 
 *    Sends data packet reliably , by trying 
 *    to send it multiple times before giving up
 *
 *  Parameters: 
 *      dest_id - destination id 
 *
 *      seq_n - sequence number 
 *
 *      cid - connection id 
 *
 *      send_id - sender id 
 *
 *      buffer - packet buffer 
 *
 *      mtu_p  - possible mtu
 *
 *
 *
 *
 * ******************************************* 
 */
int send_data_packet_reliable(int  dest_id , int  seq_n  ,int  cid, int send_id ,void * buffer , int mtu_p) 
{
 
    int retry_count = 0 ; 
   
    while ( retry_count < RETRY_COUNT )
    { 
          send_data_packet( dest_id , seq_n  , cid, send_id , buffer , mtu_p );
          usleep(WAIT_TIME_ACK);   
	  retry_count++; 
   
          if ( sender_buffer[send_id].response_code == CONTROL_ACK &&  sender_buffer[send_id].seq_no == seq_n)
          {
                break; 
                  
          } 
    }
    if ( retry_count >=   RETRY_COUNT )
    {
         return FALSE;
    
    }
    return TRUE;
}

/*
 * ******************************************* 
 *  Function: send_fin_packet_reliable 
 *
 *
 *  Description: 
 *    Sends fin packet reliably , by trying 
 *    to send it multiple times before giving up
 *
 *  Parameters: 
 *      dest_id - destination id 
 *
 *      seq_n - sequence number 
 *
 *      cid - connection id 
 *
 *      send_id - sender id 
 *
 *
 * ******************************************* 
 */

int send_fin_packet_reliable(int  dest_id ,int  cid, int send_id ) 
{
 
    int retry_count = 0 ; 
   
    while ( retry_count < RETRY_COUNT )
    { 
          send_fin_request( dest_id , cid, send_id);
          usleep(WAIT_TIME_ACK);   
	  retry_count++; 
   
          if ( sender_buffer[send_id].response_code == CONTROL_ACK &&  sender_buffer[send_id].seq_no == 0 )
          {
                break; 
                  
          } 
    }
    if ( retry_count >=   RETRY_COUNT )
    {
         return FALSE;
    
    }
    return TRUE;
}



/*
 * ******************************************* 
 *  Function: handle_application_packet 
 *
 *  Description : handles an application 
 *  layer buffer, ( it can be huge ! , 
 *  megabytes or gigabytes)  
 *
 *  Parameters: 
 *       len - length of buffer 
 *       buffer - app layer buufer 
 *
 *       dest_id - destionation id 
 *
 *
 *
 * ******************************************* 
 */

void handle_application_packet( void * buffer, int len , int dest_id)
{



        int send_id ;    
        int seq_n  =  0;     
        int mtu_p = 100; 
        int total_p = 0; 
	int count  = 0 ;
        int cid  = 0 ;
	int send_len = len;
	send_id = allocate_sender();
        printf("\n sender id = %d"  , send_id); 	
        send_connect_request(dest_id,send_id);
        usleep(WAIT_TIME_ACK);   
	if ( sender_buffer[send_id].response_code == CONTROL_CONNECT_ACK )
	{
                printf("\n  Connect  ack received ! " ); 
                mtu_p = sender_buffer[send_id].mtu_possible;  
                cid =  sender_buffer[send_id].connection_id; 
	        printf("\n CID = %d " , cid); 
	        printf("\n window size = %d " ,mtu_p); 	
	}
	seq_n++;
	total_p =   len / mtu_p ; 
	if ( len % mtu_p != 0 )
	{
		total_p++;
	}
	
	for ( count  = 1 ; count  <= total_p ; count++)
	{
		send_len -= mtu_p;
		if ( send_len >= 0 )
		
		{
	          
		   if ( send_data_packet_reliable( dest_id , seq_n  , cid, send_id , buffer , mtu_p ) )
		   {
		       buffer =  (char *)buffer +  mtu_p; 

		   }
		   else
		   {
		       fprintf(stderr, "\n Unable send the data packet reliably after many retry's , something terribly wrong !\n Please retry after sometime \n ");

		         
		   }
		      
		}
		else 
		{
		   if ( send_data_packet_reliable( dest_id , seq_n  , cid, send_id , buffer ,send_len+ mtu_p ) == FALSE )
		   {

		 
		       fprintf(stderr, "\n Unable send the data packet reliably after many retry's , very unreliable data path , i can't help it !\n Please retry after sometime \n ");

		   }
		         
		  
		
		}
           	seq_n++;


	}
	if ( send_fin_packet_reliable(dest_id , cid , send_id) == FALSE )
	{

	    fprintf(stderr, "\n Unable send the fin packet reliably after many retry's , very unreliable data path , can't help it !\n Please retry after sometime \n ");

	}
	
       deallocate_sender(send_id);   
       send_close_request(dest_id  ,  cid  , send_id);
           	  
               


}

