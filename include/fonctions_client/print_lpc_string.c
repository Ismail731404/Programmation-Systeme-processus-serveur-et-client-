#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "print_lpc_string.h"

int print_lpc_string (void *data){

	lpc_string *shm = data;	
	char *s = malloc(sizeof(char) * shm->slen);
	memcpy(s, shm->string, sizeof(char) * shm->slen);

	printf("%s\n", s);
	free(s);
	return 0;
}	
