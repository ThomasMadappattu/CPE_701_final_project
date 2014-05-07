#ifndef _STATEH_H_
#define _STATEH_H_

#include <semaphore.h>
struct state_inf
{
     int state; 
     sem_t lock;    

};

typedef struct state_inf STATE_T; 

void init_state( STATE_T *state);
void set_state(STATE_T *state , int value);
int get_state(STATE_T *state); 
  
#endif
