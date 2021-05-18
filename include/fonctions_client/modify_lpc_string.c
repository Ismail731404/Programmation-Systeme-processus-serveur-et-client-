#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "modify_lpc_string.h"

int modify_lpc_string (void *data){

    lpc_string *shm = data;	
    int error = 0;
    char mod[20]="chaine modifee";
    if(strlen(mod)<= shm->slen){
        memcpy(shm->string, mod, shm->slen);
        sleep(15);
        return 0;
    }
    else
    {
        memcpy(&shm->slen, &error, sizeof(int));
        return ENOMEM;
    }
}	
