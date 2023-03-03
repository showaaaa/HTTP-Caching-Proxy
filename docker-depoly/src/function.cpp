#include "function.h"

void P(sem_t *sem) 
{
    if (sem_wait(sem) < 0){

    }
	// unix_error("P error");
    //to log
}

void V(sem_t *sem) 
{
    if (sem_post(sem) < 0){
        
    }
	// unix_error("V error");
    //to log
}

