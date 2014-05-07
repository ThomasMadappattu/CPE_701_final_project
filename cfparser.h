#ifndef            _CFPARSER_H_ 
#define            _CFPARSER_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <ctype.h>
#include <string.h>


#define    NODE_MAX         255 
#define    NEIGHBOUR_SIZE    7 
#define    MAX_CFG_ITEMS    24
#define    NOT_FOUND        -1
#define    LINE_MAX_LEN         255    

enum
{

     FALSE ,
     TRUE 

};

struct cfg_info
{
      int node_id; 
      char node_name[NODE_MAX];
      int udp_port;        
      int neighbours[NEIGHBOUR_SIZE];    
      int total_neighbours; 
      int mtu_size;              
        
}; 
typedef struct cfg_info CFG_T; 
typedef struct cfg_info *CFG_P; 


extern CFG_T cfg_items[MAX_CFG_ITEMS]; 
extern int  __current_node; 
extern  int read_config_file(char * file); 
extern void exit_error(const char *message); 
extern int find_node_by_id( int node_id );
extern int find_node_by_name(char *name);



#endif  

