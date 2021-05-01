#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "modify_lpc_string.h"

int modify_lpc_string (void *data){

	lpc_string *shm = data;	
	char *s1 = malloc(shm->slen);
	memcpy(s1, shm->string, shm->slen);

    char *s2 = malloc(shm->slen);
    strcpy(s2, "salut");

    memcpy(shm->string, s2, shm->slen);

    free(s1);
    free(s2);
	return 0;
}	
