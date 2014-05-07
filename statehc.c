/*
 * ******************************************* 
 *   File : stateh.c 
 *   Description : state mangement 
 *
 *   Authors: Sandeep Mathew & Mahamadur Kahn
 *
 * ******************************************* 
 */

#include "stateh.h"



/*
 * ******************************************* 
 *   Function: init_state 
 *
 *   Description : intilizes the state struct
 * ******************************************* 
 */
void init_state(STATE_T *state)
{
      state->state = 0 ; 
      sem_init(&(state->lock),0,1);  

}

/*
 * ******************************************* 
 *   Function: set_state 
 *
 *   Description: sets the state struct 
 *
 * ******************************************* 
 */

void set_state(STATE_T *state , int value)
{
    sem_wait(&(state->lock));
    state->state = value;
    sem_post(&(state->lock)); 

} 
/*
 * ******************************************* 
 *   Function: get_state 
 *
 *   Description : returns the current state 
 *
 * ******************************************* 
 */
int get_state( STATE_T *state)
{
    return state->state; 
  
}


#ifdef __TEST__



#include <stdio.h> 
int main ()
{
    STATE_T state ;
    init_state(&state);
    set_state(&state,3);
    printf("\n Current state value = %d " , get_state(&state)); 
    return 0;   

}


#endif
