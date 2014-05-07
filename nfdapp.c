/*
 *  *******************************************
 *  File: nfdapp.c 
 *  Description: 
 *  This file represents the main nfd app.            
 *  The non blocking file download app 
 *  Authors: Sandeep Mathew & Mahamadur Khan  
 * 
 *
 *  *******************************************
 *
 */


#include "nfdapp.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h> 
#include <string.h>
#include "linkl.h" 
#include <time.h>
#include "transportl.h"
#include "packeth.h"



/*
 * *******************************************
 *
 * Function : get_file_size
 * Description: 
 *             clibrary way to get size of a 
 *              file
 *  Parameters:
 *          filename -  name of the file  
 *
 *  Returns:
 *           size of the file in bytes
 *
 *
 *
 ******************************************* 
*/
unsigned long get_file_size(char *filename)
{
    unsigned int file_size ;
    FILE *fp = NULL;
    fp =  fopen(filename,"r");
    if ( fp == NULL ) 
    {

       return 0; 
    }
    fseek(fp,0L,SEEK_END);
    file_size = ftell(fp);
    fclose(fp);
    return file_size;
}
/*
 * *******************************************
 *  Function:
 *            nfd_buffer_receved
 *
 *  Description:
 *           this functions handles an
 *           application buffer. The app   
 *           buffer has the format  
 *           [CODE][SIZE][FILNAME][DATA].   
 *           Depending on the application 
 *           code , appropriate action needs
 *           to be taken.
 *  Parameters:
 *           buffer -  The application buffer 
 *
 * *******************************************
 */
void nfd_buffer_receved(void *buffer , int len )
{

    int*  packet_type ;
    char *file_name; 
    void *file_buff; 
    int* file_size;
    int src_file_size;   
    int code= CODE_FAILURE;
    int *dest_id;  
    FILE *fp;

    packet_type =(int *) get_packet_item(buffer,CODE_OFFSET,CODE_SIZE); 
    switch( *packet_type)
    {

            case CODE_GET:  
                 file_name = (char *) get_packet_item(buffer, FILENAME_OFFSET , FILENAME_SIZE); 
		 src_file_size = get_file_size(file_name); 
		 file_buff = malloc(src_file_size + APP_HEADER_SIZE); 
		 dest_id = (int *)get_packet_item(buffer ,SRC_ID_OFFSET , SRC_ID_SIZE ); 
                 if ( src_file_size  > 0 ) 
		 {
                     code = CODE_SUCCESS;   
		     pack_into_packet(file_buff, CODE_OFFSET ,&code , CODE_SIZE ) ; 
		     pack_into_packet(file_buff, SIZE_OFFSET , &src_file_size , SIZE_SIZE); 
		     pack_into_packet(file_buff, FILENAME_OFFSET , file_name , strlen(file_name) + 1);
		     fprintf(stderr , "\n  reading file   %s and sending to  %d  size = %d  " , file_name , *dest_id , src_file_size); 
		     fp = fopen(file_name,"r"); 
		     fread((char*)file_buff + APP_HEADER_SIZE , src_file_size , 1 ,fp);
		     fclose(fp); 

		 }
		 else 
		 {
		     code = CODE_FAILURE;    
		     pack_into_packet(file_buff, CODE_OFFSET ,&code , CODE_SIZE ) ; 
		     pack_into_packet(file_buff, SIZE_OFFSET , &src_file_size , SIZE_SIZE); 
		     pack_into_packet(file_buff, FILENAME_OFFSET , file_name , strlen(file_name) + 1); 

		 
		 }
		 nfd_send_buffer_transport(file_buff, src_file_size + APP_HEADER_SIZE, *dest_id); 
                 break;

	    case CODE_SUCCESS:

                 file_name = (char *) get_packet_item(buffer, FILENAME_OFFSET , FILENAME_SIZE); 
		 file_size = (int *) get_packet_item(buffer , SIZE_OFFSET , SIZE_SIZE); 
                 fp = fopen(file_name , "w"); 
                 fwrite((char *)buffer + APP_HEADER_SIZE , *file_size , 1 , fp);  
                 fclose(fp);     
		 fprintf(stderr , "\n File successfully written!!");  
		 break;

	    case CODE_FAILURE:
	         file_name = (char *) get_packet_item(buffer, FILENAME_OFFSET , FILENAME_SIZE); 
    	         fprintf(stderr , "\n Unable to download file  %s" , file_name);
                 break;  
            default:
		    fprintf(stderr, "\n Unknown packet type = %d " , *packet_type); 
		    break; 
    
    
    }
          
 
}
/*
 * *******************************************
 *  Function:
 *           nfd_send_buffer_transport 
 *  Description:
 *           Sends the application buffer  
 *           to transport layer 
 *
 *  Parameters:
 *           buffer - Application buffer 
 *           len    - length of the buffer
 *           dest_id - destination node id 
 *
 *
 *
 * *******************************************
 */


void nfd_send_buffer_transport(void *buffer , int len, int dest_id)
{

	pthread_t thread_handle;  
        SND_ARGS_T snd = malloc(sizeof ( struct snd_args )); 
        snd->buffer = buffer;
	snd->len    = len ; 
	snd->dest_id = dest_id; 
       	pthread_create(&thread_handle , NULL , sender_thread , (void *)snd); 

}

/*
 * *******************************************
 *  Function:
 *            sender_thread 
 *
 *  Description:
 *            Starts the a thread to send     
 *            and entire application buffer 
 *
 *  Parameters:
 *            args - structure containing 
 *            buffer , length and destination
 *            id
 *
 *
 * *******************************************
 *
 */
void* sender_thread(void *args)
{
  
	SND_ARGS_T snd_args = (SND_ARGS_T ) args ; 
        handle_application_packet(snd_args->buffer, snd_args->len,snd_args->dest_id);   
        return NULL;
}
/*
 * *******************************************
 *   Function:
 *            handle_start_server
 *    Description:
 *            handle the start_server 
 *            command 
 *
 *    Parameters:
 *            itc_file - Name of config file 
 *            node_id - current node id 
 *            inp_pointer - pointer to input
 *            string 
 *            thread - id of thread to be 
 *            created 
 *
 * *******************************************
 *
 */
void handle_start_server(char *itc_file  , int node_id , char *inp_pointer, pthread_t *thread)
{

  read_config_file(itc_file);   
  pthread_create(thread,NULL ,server_thread, &node_id);
  init_transport_layer(); 

  
   

}
/*
 * *******************************************
 *  Function:
 *            handle_input 
 *
 *  Description:
 *       takes action as per the  user   
 *       enter command  
 *
 *
 *  Parameters:
 *      input  - pointer to input string     
 *      itc_file - name of config file 
 *      node_id - id of the current node 
 *
 *
 *
 *
 * *******************************************
 */

void handle_input(char *input, char *itc_file , int node_id)
{

      int quit_len;
      char *inp_ptr; 
      pthread_t thread;  
      int srv_len  = strlen("start_service");
      int exit_len = strlen("exit");   
      int stop_srv_len  = strlen("stop_service"); 
      int connect_len  = strlen("connect");
      int download_len = strlen("download");
      int dump_len    = strlen("route_table");
      int set_gar_len  =  strlen("set_garbler");
      int link_up_len   = strlen("link_up"); 
      int link_down_len  = strlen("link_down");  
      char file_name[NAME_MAX];
      int max_connections; 
      char *app_buffer= (char *)malloc(APP_HEADER_SIZE);
      int code;  
      int dest_node;
      int window_size;    

        

      quit_len  = strlen("quit");
      inp_ptr   = input; 
      srand(time(NULL));   
      if ( strncmp(input,"quit",quit_len) == 0 )
      {
            printf("\n Quiting ...  \n"); 
            exit(EXIT_SUCCESS);  

      }
      if ( strncmp(input,"exit",exit_len) == 0 )
      {
            printf("\n Quiting ...  \n"); 
            exit(EXIT_SUCCESS);  

      } 
      if ( strncmp(input,"start_service",srv_len) == 0 )
      {
            printf("\n Starting Server Thread : SID  = %d \n"  , rand()% 10000 );  
            inp_ptr += srv_len ;
	    sscanf(inp_ptr,"%d", &max_connections); 
            printf("\n Max connections = %d \n" , max_connections ); 
	    init_state(&__max_connections); 
	    set_state(&__max_connections,max_connections);
	    handle_start_server(itc_file , node_id , inp_ptr , &thread);   
      }     
      if ( strncmp(input,"stop_service",stop_srv_len) == 0 )
      {
           printf("\n  stoping the serivce  ... \n");      
           pthread_cancel(thread);     

      }	      
      if ( strncmp(input,"connect",connect_len) == 0)
      {

              
	      inp_ptr +=  connect_len;
	      sscanf(inp_ptr,"%d",&dest_node);
	      sscanf(inp_ptr,"%d",&window_size);          
	      nfd_send_buffer_transport("Donkey!" ,8,2); 

      
      }   
      if ( strncmp(input,"route_table",dump_len) == 0)
      {
           dump_route_table();    
	   printf("\n Route table dumped to file \n");  
      }
      if ( strncmp(input,"link_up",link_up_len) == 0)
      {
	   inp_ptr+=link_up_len;    
           sscanf(inp_ptr,"%d",&dest_node);  
	   link_up(dest_node); 
	   printf("\n Link upped :) \n");  
      }
      if ( strncmp(input,"link_down",link_down_len) == 0)
      {
           inp_ptr += link_down_len; 
           sscanf(inp_ptr,"%d",&dest_node); 	   
           link_down(dest_node); 
	   printf("\n Link downed!\n");  
      }
      if ( strncmp(input,"download",download_len) == 0 )
      {

	     inp_ptr += download_len;  
             sscanf(inp_ptr , "%d%s" , &dest_node , file_name); 
             code = CODE_GET ; 	     
	     pack_into_packet(app_buffer, CODE_OFFSET , &code , CODE_SIZE); 
             pack_into_packet(app_buffer ,FILENAME_OFFSET , file_name , strlen(file_name) +1 );
	     printf("\n Currnet node = %d "  , __current_node );
	     pack_into_packet(app_buffer , SRC_ID_OFFSET , &__current_node , SRC_ID_SIZE);
	     nfd_send_buffer_transport(app_buffer,APP_HEADER_SIZE,dest_node); 

      }  
      if ( strncmp(input,"set_garbler",set_gar_len) == 0)
      {

              
	      inp_ptr +=  connect_len;
	      sscanf(inp_ptr,"%d%d",&__loss_frac , &__corr_fact);
	      fprintf(stderr ,"\n garbler charecteristics set ... \n");         

      
      }
}

/*
 * *******************************************
 *   Function:
 *         Main 
 *   Description:
 *       Main program , starts a shell 
 *       and accepts user commands.  
 *   Parameters:
 *       argv - current node name and itc 
 *       script file     
 *
 *
 * *******************************************
 *
 */
int main( int argc , char *argv[])
{
    
    const char * prompt="nfd>";
    char user_inp[BUF_MAX];  
    int current_node_id  ;  
    if ( argc <=  2 )
    { 
           printf("\n Usage : %s <nodeid>  itc_script_file " , argv[0]);
           exit(EXIT_FAILURE);    

    }
    
    memset(user_inp,0 , BUF_MAX);    
    current_node_id  = atoi(argv[1]);
    __current_node = current_node_id;
    init_transport_layer();  
    while (TRUE )
    {
        printf("%s" , prompt); 
        fgets(user_inp , BUF_MAX-2 ,stdin);
        handle_input(user_inp , argv[2] , current_node_id);   
        memset(user_inp,0 , BUF_MAX);    
          
    }   
}
