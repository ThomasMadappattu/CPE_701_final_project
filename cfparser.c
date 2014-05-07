/*
 * ******************************************* 
 *  File : cfparser.c 
 *
 *  Description: configuration parser 
 *
 *
 *  Authors : Sandeep Mathew & Mahmadur Khan  
 *
 * ******************************************* 
 */


#include "cfparser.h"
CFG_T cfg_items[MAX_CFG_ITEMS]; 
int  __cfg_size =0 ; 
int  __current_node =0 ; 


/*
 * ******************************************* 
 *  Function: skip_space 
 *
 *  Description : skips space in line bufer 
 *
 * ******************************************* 
 */

static void skip_space(char **line_ptr ,int *line_index, int buf_len )
{
           
           char *ptr = *line_ptr; 
           while(isspace(*ptr) && *ptr != '\0' && *ptr != EOF && *line_index < buf_len)
           {
               ptr++;
               (*line_index)++;  
           }
           *line_ptr = ptr;   
} 
/* 
 *  ******************************************* 
 *   Function: skip_to_space 
 *
 *   Description : skips to space in line buffer 
 *
 *  ******************************************* 
 */


static void skip_to_space(char **line_ptr ,int *line_index, int buf_len )
{
          
           char *ptr = *line_ptr;  
           while(!isspace(*ptr) && *ptr != '\0' && *ptr != EOF && *line_index < buf_len)
           {
               ptr++;
               (*line_index)++;  
           }
           *line_ptr  = ptr;  
} 
/*
 * *******************************************  
 *  Function: read_config_file 
 *
 *  Description : reads up the config files 
 *  into a structre , shared across the pro
 *  gram
 *
 * ******************************************* 
 */

int read_config_file(char *file)
{
       FILE *fp; 
       int index  = 0 ;      
       int id ; 
       char line[LINE_MAX_LEN];
       char *line_ptr;
       int line_index  = 0 ; 
       int nindex=0;      
       fp = fopen(file , "r");
       if ( fp == NULL ) 
       { 
            perror("Error opening file " ) ; 
            exit(EXIT_FAILURE);  
       }    
       fgets(line, LINE_MAX_LEN-2,fp); 
       
       while(!feof(fp))
       {
           line_ptr = line; 
           skip_space(&line_ptr , &line_index, LINE_MAX_LEN);         
           sscanf(line_ptr,"%d",&id);
           cfg_items[index].node_id = id;
           skip_to_space(&line_ptr,&line_index,LINE_MAX_LEN);
           skip_space(&line_ptr,&line_index,LINE_MAX_LEN);   
           sscanf(line_ptr,"%s",cfg_items[index].node_name);  
           skip_to_space(&line_ptr,&line_index,LINE_MAX_LEN); 
           skip_space(&line_ptr,&line_index,LINE_MAX_LEN);  
           sscanf(line_ptr,"%d" , &cfg_items[index].udp_port);
           
           while(TRUE )
           {
                 id = -1; 
                 skip_to_space(&line_ptr,&line_index,LINE_MAX_LEN); 
                 skip_space(&line_ptr,&line_index,LINE_MAX_LEN);  
                 sscanf(line_ptr,"%d",&id);
                 if ( id != -1 )
                 {    
                     cfg_items[index].neighbours[nindex++] = id;
                 }   
                 else
                 {
                      break; 
                 }    
                  
           }                             
           cfg_items[index].mtu_size = cfg_items[index].neighbours[nindex-1];
           nindex--;
           cfg_items[index].total_neighbours = nindex;  
           index++;           
           nindex = 0 ; 
           line_index = 0 ;
           printf("%s" , line_ptr);  
           fgets(line, LINE_MAX_LEN-2,fp); 
    
       }                            
       __cfg_size  =  index ; 
       return index;      
}

/*
 * ******************************************* 
 *   Function: find_node_by_id
 *
 *   Description : finds node by id 
 *
 *
 * ******************************************* 
 */
int find_node_by_id( int node_id )
{
   int count; 
   for ( count = 0 ; count   < __cfg_size ; count++)
   {
       if (cfg_items[count].node_id == node_id)
            return count;
                                  
   }

   return NOT_FOUND; 
}

/*
 * ******************************************* 
 *   Function: find_node_by_name
 *
 *   Description : finds node by name 
 *
 *
 * ******************************************* 
 */

int find_node_by_name(char *name)
{

     int count = 0 ; 
     for ( count = 0 ; count < __cfg_size ; count++ ) 
     {
           if ( strcmp(cfg_items[count].node_name,name)  == 0 ) 
                 return count;  
     
     }
     return NOT_FOUND; 

}


#ifdef __TEST__ 

int main()
{


         int total_items , index , nindex;
         total_items = read_config_file("test.txt");
         for (index  = 0  ; index < total_items ; index ++ ) 
         {
             printf("\n id = %d name = %s  port = %d mtu = %d " , cfg_items[index].node_id , cfg_items[index].node_name , cfg_items[index].udp_port,cfg_items[index].mtu_size);
             for ( nindex = 0 ; nindex < cfg_items[index].total_neighbours ; nindex++)
             {
                 printf(" \n \t  %d " , cfg_items[index].neighbours[nindex]);

             }  

         }   



         return 0 ;

}
#endif 
