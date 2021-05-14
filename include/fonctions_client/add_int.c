#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "add_int.h"

int add_int(void *data){
	int *a = malloc(sizeof(int));
	int *b = malloc(sizeof(int));

	memcpy(a, data, sizeof(int));
	memcpy(b, (char *) data + sizeof(int), sizeof(int));
	
	int result = *a + *b;
	memcpy(data, &result, sizeof(int));

	return 0;
}

