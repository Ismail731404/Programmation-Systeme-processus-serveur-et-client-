#include "../../include/lpc_utility.h"
#include "../../include/lpc_structure.h"
#include "../../include/lpc_functions/lpc_functions.h"
#include "add_int.h"

int add_int(void *memory){

	void *data = (void *)((char *)memory + sizeof(header));

	int *a = malloc(sizeof(int));
	int *b = malloc(sizeof(int));

	memcpy(a, data, sizeof(int));
	memcpy(b, (char *) data + sizeof(int), sizeof(int));
	
	int result = *a + *b;
	memcpy(data, &result, sizeof(int));
	//sleep(15);
	return 0;
}

