#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "modify_lpc_string.h"

int modify_lpc_string (void *data){

    lpc_string *shm = data;	
    
    char *s = malloc(shm->slen);
    strcpy(s, "salut");
    memcpy(shm->string, s, shm->slen);

    free(s);
    return 0;
}	
